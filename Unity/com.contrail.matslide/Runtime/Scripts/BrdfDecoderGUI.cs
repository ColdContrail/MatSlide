using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using Microsoft.ML.OnnxRuntime;
using Microsoft.ML.OnnxRuntime.Tensors;

public class BrdfDecoderGUI : MonoBehaviour
{
    public TextAsset modelAsset;
    public Material targetMaterial;

    float[] inputValues = new float[10];
    string status = "就绪";

    InferenceSession session;
    TextAsset loadedAsset;
    Texture3D m_OutputTexture;

    bool isExpanded = true;
    float panelAnim; // 0=collapsed, 1=expanded

    // --- Style ---
    GUISkin skin;
    bool styleBuilt;
    Color bgColor = new Color(0.1f, 0.11f, 0.13f, 0.94f);
    Color panelColor = new Color(0.08f, 0.09f, 0.10f, 1f);
    Color accentColor = new Color(0.3f, 0.48f, 0.72f, 1f);
    Color textColor = new Color(0.86f, 0.88f, 0.90f, 1f);

    void BuildSkin()
    {
        if (styleBuilt) return;
        skin = Instantiate(GUI.skin);

        skin.window.normal.background = Make2x2Texture(bgColor);
        skin.window.padding = new RectOffset(14, 14, 14, 14);
        skin.window.border = new RectOffset(8, 8, 8, 8);

        skin.box.normal.background = Make2x2Texture(panelColor);
        skin.box.padding = new RectOffset(12, 12, 10, 10);
        skin.box.border = new RectOffset(6, 6, 6, 6);
        skin.box.normal.textColor = textColor;
        skin.box.fontSize = 13;
        skin.box.fontStyle = FontStyle.Bold;

        skin.label.normal.textColor = textColor;
        skin.label.fontSize = 12;

        skin.button.normal.background = Make2x2Texture(accentColor * 0.8f);
        skin.button.hover.background = Make2x2Texture(accentColor);
        skin.button.active.background = Make2x2Texture(accentColor * 0.65f);
        skin.button.normal.textColor = Color.white;
        skin.button.hover.textColor = Color.white;
        skin.button.fontSize = 14;
        skin.button.fontStyle = FontStyle.Bold;
        skin.button.padding = new RectOffset(12, 12, 8, 8);
        skin.button.border = new RectOffset(6, 6, 6, 6);

        skin.horizontalSlider.normal.background = Make2x2Texture(new Color(0.16f, 0.17f, 0.20f, 1f));
        skin.horizontalSlider.hover.background = Make2x2Texture(new Color(0.20f, 0.21f, 0.25f, 1f));
        skin.horizontalSlider.fixedHeight = 6f;
        skin.horizontalSlider.border = new RectOffset(0, 0, 0, 0);

        skin.horizontalSliderThumb.normal.background = Make2x2Texture(accentColor);
        skin.horizontalSliderThumb.hover.background = Make2x2Texture(accentColor * 1.15f);
        skin.horizontalSliderThumb.active.background = Make2x2Texture(accentColor * 0.85f);
        skin.horizontalSliderThumb.fixedWidth = 12f;
        skin.horizontalSliderThumb.fixedHeight = 12f;
        skin.horizontalSliderThumb.border = new RectOffset(4, 4, 4, 4);

        styleBuilt = true;
    }

    Texture2D Make2x2Texture(Color c)
    {
        var t = new Texture2D(2, 2);
        Color[] px = { c, c, c, c };
        t.SetPixels(px);
        t.Apply();
        t.hideFlags = HideFlags.HideAndDontSave;
        return t;
    }

    bool EnsureSession()
    {
        if (session != null && modelAsset == loadedAsset)
            return true;

        CleanupSession();

        if (modelAsset == null)
        {
            status = "模型资源未设置";
            return false;
        }

        var opts = new SessionOptions();
        opts.GraphOptimizationLevel = GraphOptimizationLevel.ORT_ENABLE_ALL;
        session = new InferenceSession(modelAsset.bytes, opts);
        loadedAsset = modelAsset;
        Debug.Log($"ONNX Runtime loaded: {modelAsset.name}");
        status = "模型已加载";
        return true;
    }

    void RunInference()
    {
        if (!EnsureSession())
            return;

        var inputTensor = new DenseTensor<float>(inputValues, new[] { 1, 10 });
        var inputs = new List<NamedOnnxValue>
        {
            NamedOnnxValue.CreateFromTensor(session.InputMetadata.Keys.First(), inputTensor)
        };

        float t0 = Time.realtimeSinceStartup;
        using var results = session.Run(inputs);
        float elapsed = Time.realtimeSinceStartup - t0;

        var output = results.First().AsTensor<float>();
        float[] data = output.ToArray();
        var od = output.Dimensions;
        int[] dims = new int[od.Length];
        for (int i = 0; i < od.Length; i++) dims[i] = od[i];

        if (targetMaterial != null)
        {
            m_OutputTexture = BuildTexture3D(data, dims, m_OutputTexture);
            targetMaterial.SetTexture("_BrdfLUT", m_OutputTexture);
        }

        status = $"推理完成 ({elapsed * 1000:F1}ms)";
    }

    Texture3D BuildTexture3D(float[] data, int[] dims, Texture3D reuse)
    {
        int phiD   = dims[2];
        int thetaD = dims[3];
        int thetaH = dims[4];
        int strideC = phiD * thetaD * thetaH;

        const float RedScale   = 1.0f  / 1500.0f;
        const float GreenScale = 1.15f / 1500.0f;
        const float BlueScale  = 1.66f / 1500.0f;
        float[] scales = { RedScale, GreenScale, BlueScale };

        Texture3D tex = reuse;
        if (tex == null || tex.width != phiD || tex.height != thetaD || tex.depth != thetaH)
        {
            if (tex != null)
            {
                if (targetMaterial != null)
                    targetMaterial.SetTexture("_BrdfLUT", null);
                DestroyImmediate(tex);
            }
            tex = new Texture3D(phiD, thetaD, thetaH, TextureFormat.RGBAHalf, false);
            tex.wrapMode = TextureWrapMode.Clamp;
            tex.filterMode = FilterMode.Bilinear;
        }

        Color[] pixels = new Color[phiD * thetaD * thetaH];

        for (int pd = 0; pd < phiD; pd++)
        {
            for (int td = 0; td < thetaD; td++)
            {
                for (int th = 0; th < thetaH; th++)
                {
                    int flatIdx = pd * thetaD * thetaH + td * thetaH + th;
                    float r = Mathf.Max(0f, data[flatIdx]              * scales[0]);
                    float g = Mathf.Max(0f, data[strideC + flatIdx]    * scales[1]);
                    float b = Mathf.Max(0f, data[2 * strideC + flatIdx] * scales[2]);
                    int pixelIdx = pd + td * phiD + th * phiD * thetaD;
                    pixels[pixelIdx] = new Color(r, g, b, 1f);
                }
            }
        }

        tex.SetPixels(pixels);
        tex.Apply();
        return tex;
    }

    void CleanupSession()
    {
        session?.Dispose();
        session = null;
        loadedAsset = null;
    }

    void OnGUI()
    {
        BuildSkin();
        GUI.skin = skin;

        float target = isExpanded ? 1f : 0f;
        panelAnim = Mathf.Lerp(panelAnim, target, Time.deltaTime * 12f);
        if (Mathf.Abs(panelAnim - target) < 0.001f)
            panelAnim = target;

        float panelW = 340f;
        float panelH = 560f;

        // --- Collapsed: small floating button ---
        if (panelAnim < 0.01f && !isExpanded)
        {
            Rect toggleBtn = new Rect(16, 16, 44, 44);
            if (GUI.Button(toggleBtn, "◈", skin.button))
            {
                isExpanded = true;
                RunInference();
            }
            return;
        }

        if (panelAnim < 0.01f) return;

        GUI.color = new Color(1, 1, 1, panelAnim);

        Rect panel = new Rect(16, 16, panelW, panelH);
        GUI.Box(panel, "", GUI.skin.window);
        GUILayout.BeginArea(new Rect(30, 30, panelW - 28, panelH - 28));

        // Title bar with collapse button
        GUILayout.BeginHorizontal();

        GUIStyle titleStyle = new GUIStyle(GUI.skin.label)
        {
            fontSize = 16,
            fontStyle = FontStyle.Bold,
            alignment = TextAnchor.MiddleLeft,
            normal = { textColor = new Color(0.9f, 0.91f, 0.93f, 1f) }
        };
        GUILayout.Label("BRDF Decoder", titleStyle);

        GUILayout.FlexibleSpace();

        GUIStyle collapseBtnStyle = new GUIStyle(GUI.skin.button)
        {
            fontSize = 12,
            fontStyle = FontStyle.Bold,
            fixedWidth = 26,
            fixedHeight = 26,
            padding = new RectOffset(0, 0, 2, 0),
            normal = { background = Make2x2Texture(new Color(1, 1, 1, 0.08f)), textColor = new Color(0.5f, 0.52f, 0.56f, 1f) },
            hover = { background = Make2x2Texture(new Color(1, 1, 1, 0.18f)), textColor = textColor }
        };
        if (GUILayout.Button("−", collapseBtnStyle))
            isExpanded = false;

        GUILayout.EndHorizontal();

        GUIStyle subtitleStyle = new GUIStyle(GUI.skin.label)
        {
            fontSize = 10,
            alignment = TextAnchor.MiddleCenter,
            normal = { textColor = new Color(0.5f, 0.52f, 0.56f, 1f) }
        };
        GUILayout.Label("ONNX Runtime Inference", subtitleStyle);
        GUILayout.Space(10);

        // Model info
        GUILayout.BeginHorizontal();
        GUILayout.Label("◈", GUI.skin.label, GUILayout.Width(18));
        GUILayout.Label($"模型: {modelAsset?.name ?? "未设置"}");
        GUILayout.EndHorizontal();
        GUILayout.Space(8);

        DrawDivider();

        GUIStyle sectionStyle = new GUIStyle(GUI.skin.label)
        {
            fontSize = 11,
            fontStyle = FontStyle.Bold,
            normal = { textColor = new Color(0.65f, 0.67f, 0.72f, 1f) }
        };
        GUILayout.Label("输入参数", sectionStyle);
        GUILayout.Space(6);

        DrawParamSlider(0, "GGX Roughness", 0f, 1f);
        GUILayout.Space(4);

        DrawDivider();
        GUILayout.Space(2);

        GUILayout.Label("Diffuse Albedo", sectionStyle);
        GUILayout.Space(2);
        DrawParamSlider(1, "  R", 0f, 0.5f);
        DrawParamSlider(2, "  G", 0f, 0.5f);
        DrawParamSlider(3, "  B", 0f, 0.5f);
        GUILayout.Space(4);

        DrawDivider();
        GUILayout.Space(2);

        GUILayout.Label("Specular Albedo", sectionStyle);
        GUILayout.Space(2);
        DrawParamSlider(4, "  R", 0f, 0.5f);
        DrawParamSlider(5, "  G", 0f, 0.5f);
        DrawParamSlider(6, "  B", 0f, 0.5f);
        GUILayout.Space(4);

        DrawDivider();
        GUILayout.Space(2);

        GUILayout.Label("Optimal Threshold", sectionStyle);
        GUILayout.Space(2);
        DrawParamSlider(7, "  R", 0f, 1f);
        DrawParamSlider(8, "  G", 0f, 1f);
        DrawParamSlider(9, "  B", 0f, 1f);

        GUILayout.Space(6);
        DrawDivider();
        GUILayout.Space(6);

        // Action buttons
        GUILayout.BeginHorizontal();
        GUILayout.FlexibleSpace();

        if (GUILayout.Button("重 置", GUILayout.Width(90), GUILayout.Height(30)))
        {
            for (int i = 0; i < 10; i++) inputValues[i] = 0f;
            status = "已重置";
        }

        GUILayout.Space(10);

        GUIStyle inferBtn = new GUIStyle(GUI.skin.button)
        {
            fontSize = 14,
            fontStyle = FontStyle.Bold,
            normal = { background = Make2x2Texture(accentColor), textColor = Color.white },
            hover = { background = Make2x2Texture(accentColor * 1.15f), textColor = Color.white }
        };
        if (GUILayout.Button("▶ 推 理", inferBtn, GUILayout.Width(110), GUILayout.Height(30)))
            RunInference();

        GUILayout.FlexibleSpace();
        GUILayout.EndHorizontal();

        GUILayout.Space(6);
        DrawDivider();
        GUILayout.Space(4);

        // Status
        GUIStyle statusStyle = new GUIStyle(GUI.skin.label)
        {
            fontSize = 11,
            alignment = TextAnchor.MiddleCenter
        };
        Color statusColor = status.StartsWith("推理完成") ? new Color(0.3f, 0.8f, 0.4f, 1f)
                          : status.Contains("未设置") ? new Color(0.9f, 0.35f, 0.35f, 1f)
                          : new Color(0.55f, 0.58f, 0.62f, 1f);
        statusStyle.normal.textColor = statusColor;

        GUILayout.BeginHorizontal();
        GUILayout.FlexibleSpace();
        GUILayout.Label("●", statusStyle, GUILayout.Width(14));
        GUILayout.Label(status, statusStyle);
        GUILayout.FlexibleSpace();
        GUILayout.EndHorizontal();

        GUILayout.EndArea();

        GUI.color = Color.white;
    }

    void DrawParamSlider(int index, string label, float min, float max)
    {
        GUILayout.BeginHorizontal();
        GUILayout.Space(4);

        GUIStyle labelStyle = new GUIStyle(GUI.skin.label)
        {
            fontSize = 11,
            fixedWidth = 110,
            normal = { textColor = textColor }
        };
        GUILayout.Label(label, labelStyle);

        inputValues[index] = GUILayout.HorizontalSlider(inputValues[index], min, max, GUILayout.MinWidth(100));

        GUIStyle valStyle = new GUIStyle(GUI.skin.label)
        {
            fontSize = 11,
            fixedWidth = 44,
            alignment = TextAnchor.MiddleRight,
            normal = { textColor = new Color(0.55f, 0.58f, 0.62f, 1f) }
        };
        GUILayout.Label(inputValues[index].ToString("F3"), valStyle);
        GUILayout.EndHorizontal();
    }

    void DrawDivider()
    {
        Rect r = GUILayoutUtility.GetRect(GUIContent.none, GUI.skin.horizontalSlider, GUILayout.ExpandWidth(true), GUILayout.Height(1));
        Color old = GUI.color;
        GUI.color = new Color(0.18f, 0.19f, 0.22f, 0.6f);
        GUI.Box(r, "", GUI.skin.horizontalSlider);
        GUI.color = old;
    }

    void CleanOutputTexture()
    {
        if (m_OutputTexture != null)
        {
            if (targetMaterial != null)
                targetMaterial.SetTexture("_BrdfLUT", null);
            DestroyImmediate(m_OutputTexture);
            m_OutputTexture = null;
        }
    }

    void OnDisable()
    {
        CleanupSession();
        CleanOutputTexture();
    }

    void OnDestroy()
    {
        CleanupSession();
        CleanOutputTexture();
    }
}
