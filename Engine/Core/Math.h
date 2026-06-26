#pragma once

#include <DirectXMath.h>

// Lumina Engine - Core
// DirectXMath を薄く包む数学エイリアス。
// 上位コードが DirectX:: を直接書かずに済むようにし、将来差し替え余地を残す。
// ※ Phase 0 では型エイリアスのみ。演算ヘルパは必要になった時点で足す。
namespace Lumina
{
    namespace dxm = DirectX;

    using Float2 = DirectX::XMFLOAT2;
    using Float3 = DirectX::XMFLOAT3;
    using Float4 = DirectX::XMFLOAT4;

    using Matrix = DirectX::XMFLOAT4X4;

    // SIMDベクトル/行列 (計算用)
    using Vec   = DirectX::XMVECTOR;
    using Mat   = DirectX::XMMATRIX;
}
