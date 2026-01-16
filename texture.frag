#ifdef GL_ES
precision highp float;
#endif

//uniform sampler2D occSampler0; // BaseColor
//uniform sampler2D occSampler1; // Emissive
uniform sampler2D uTex0; // GL_TEXTURE0
uniform sampler2D uTex1; // GL_TEXTURE1
uniform float uTime; // GL_TEXTURE1

varying vec4 vWorldPos; // 世界空间位置
varying vec3 vNormal;
varying vec4 vColor;    // 顶点传递颜色
varying vec2 vUv;   // ✅ 新增

void main()
{
    vec4 col = occColor;
    col = mix(col, mix(texture2D(uTex0, vUv), texture2D(uTex1, vUv), abs(cos(uTime))), 0.5);
    occFragColor = col;

}
