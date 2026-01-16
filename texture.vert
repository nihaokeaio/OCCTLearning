#ifdef GL_ES
precision highp float;
#endif

// 使用 OCCT 已声明的变量
varying vec4 vWorldPos;
varying vec3 vNormal;
varying vec4 vColor;
varying vec2 vUv;

void main()
{
    // OCCT 已经提供 occVertex / occNormal / occVertColor
    vWorldPos = occModelWorldMatrix * occVertex;
    vNormal = normalize((occModelWorldMatrix * vec4(occNormal, 0.0)).xyz);
    vColor = occVertColor;
    // ✅ OCCT 在 Vertex 阶段提供 aTexCoord
    vUv = occTexCoord.xy;
    // 使用 OCCT 的默认矩阵
    gl_Position = occProjectionMatrix * occWorldViewMatrix * vWorldPos;
}
