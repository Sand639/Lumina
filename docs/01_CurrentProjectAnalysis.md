# 現プロジェクト(HG41)の棚卸しと設計評価

新エンジンをゼロから設計する前提として、既存の課題プロジェクト `HG41` に
「何が実装されているか」「設計上どこが弱いか」を整理する。
新エンジンでは **実装済みの知見は引き継ぎ、設計上の弱点は作り直す**。

---

## 1. 実装済み機能（技術資産として引き継ぐ）

### グラフィックス基盤（DirectX12）
- Device / DXGI Factory・Adapter / CommandQueue / CommandList / CommandAllocator
- SwapChain（ダブルバッファ）、Fence によるフレーム同期
- デバッグ用の基本構成

### リソース管理
- **ディスクリプタ・プール**: SRV(最大10000) / RTV(最大1000) をフリーリストで確保・返却
- **定数バッファ・リング**: フレームごとに 512byte × 1000スロット × 2フレームを使い回す
- **PSOキャッシュ**: `name → PipelineState` の `unordered_map`
- ルートシグネチャ 1本（CBV×4 + SRV×8 + StaticSampler×2）

### レンダリング機能（ここが評価ポイントの中身）
- **デファードレンダリング**: G-buffer 4枚（Color / Normal / Position / Material、すべて `R16G16B16A16_FLOAT`）
  → デファードライティング → ポスト → バックバッファ
- **PBR / BRDF**: Cook-Torrance（GGXのD項・SmithのG項・SchlickのF項）、metallic / roughness / F0
- **IBL**: equirectangular 環境マップを法線・反射ベクトルでサンプル（簡易版、mipを手動指定）
- **ポストエフェクト**: Reinhard トーンマッピング + ガンマ補正（ビネット等はコメントで用意）
- **法線マッピング**: スクリーン空間微分ベースの TBN（接線頂点が不要）+ フラット法線フォールバック + ImGuiトグル
- **ImGui**: DX12 + Win32 バックエンド

### アセット / 入力 / 数学
- OBJ / MTL ローダー（独自・バイナリキャッシュ付き）
- DDS テクスチャ読み込み（DirectXTK12 由来の `DDSTextureLoader12`）
- キーボード（DirectXTK 由来）/ XInput
- DirectXMath
- 左手座標系へ変換（ローダーで X 反転）

---

## 2. 設計上の弱点（新エンジンで作り直す）

| # | 弱点 | 何が問題か | 新エンジンでの方針 |
|---|------|-----------|------------------|
| 1 | **God-object** | `RenderManager` が1500行超で初期化・リソース・PSO・描画手順・テクスチャ・バッファの全責務を抱える | 単一責任で分割（Device / Swapchain / CommandContext / DescriptorAllocator / ResourceFactory …） |
| 2 | **シングルトン乱用** | `RenderManager`・`GameManager`・`GameEngine` が全部シングルトン。`GameManager` は未使用で `RenderManager` を値で二重保持 | 所有権を明確化。依存は注入（DI）。シングルトンは原則排除 |
| 3 | **描画手順がハードコード** | `DrawBegin`/`DrawEnd` に「G-buffer→ライティング→ポスト」が直書き。シャドウやSSAOを足すには既存関数を編集するしかない | **RenderGraph（パス＋リソース依存）** で宣言的に組む |
| 4 | **ECSが死んでいる** | `Entity/Component/Scene/Transform` という立派な土台があるのに、実シーン(`Field`/`Sky`/`TestObject`)は使わず `GameEngine` が直保持 | シーン表現を一本化し、描画は **RenderQueue（描画アイテムのsubmission）** に集約 |
| 5 | **ライトデータの重複** | 各オブジェクトの `Draw` が個別に ENV（ライト方向・色）をセット | ライトは Scene が単一の真実の源として保持 |
| 6 | **RAII抽象が薄い** | 生の `ComPtr` と `WriteToSubresource` が各所に散在。Upload管理なし | `GpuBuffer` / `GpuTexture` / `UploadContext` を RAII で抽象化 |
| 7 | **シェーダーがビルド時固定(.cso)** | 実行時コンパイル・ホットリロード・リフレクションが無く、イテレーションが遅い | **DXC で実行時コンパイル + リフレクション + ホットリロード** |
| 8 | **ディスクリプタ戦略が素朴** | フラットなフリーリストのみ | 用途別アロケータ（永続 / フレーム一時）、将来 bindless |
| 9 | **フォーマット・解像度が直値** | G-buffer / RT のフォーマットがコードに直書き | 設定構造体・RenderGraph のリソース記述で持つ |

---

## 3. 引き継ぐもの・捨てるもの（要約）

- **引き継ぐ**: PBR/BRDFの数式、デファードの構成、IBLの考え方、法線マッピング手法、ポストの式、アセット形式の知見
- **捨てる**: God-objectなRenderManager、未使用ECS、シングルトン構成、ハードコードされた描画手順
- **新しく入れる**: RHI抽象レイヤー、RenderGraph、実行時シェーダーコンパイル、RAIIリソース、データ駆動シーン
