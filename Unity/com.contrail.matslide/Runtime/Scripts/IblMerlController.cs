using UnityEngine;

[ExecuteAlways]
public class IblMerlController : MonoBehaviour
{
    [Header("BRDF Data")]
    public Texture3D brdfLUT;
    public TextAsset brdfBinaryFile;

    [Header("Environment")]
    public Cubemap envCubemap;

    [Header("Material Parameters")]
    [Range(0.01f, 1f)]
    public float roughness = 0.1f;
    [Range(0.1f, 5f)]
    public float exposure = 1f;
    [Range(1f, 50f)]
    public float maxSampleValue = 10f;

    [Header("Point Light")]
    public Light pointLight;
    public Color lightColor = Color.white;
    public float lightIntensity = 100f;
    public Vector3 lightPosition = new Vector3(2f, 3f, 1f);

    [Header("References")]
    public Material targetMaterial;
    public MeshRenderer meshRenderer;

    void OnEnable()
    {
        if (meshRenderer == null)
            meshRenderer = GetComponent<MeshRenderer>();

        if (targetMaterial == null && meshRenderer != null)
            targetMaterial = meshRenderer.sharedMaterial;

        LoadBrdfIfNeeded();
    }

    void LoadBrdfIfNeeded()
    {
        if (brdfLUT != null) return;

        if (brdfBinaryFile != null)
        {
            brdfLUT = MerlBrdfLoader.LoadBrdfTextureFromRawData(brdfBinaryFile.bytes);
        }
    }

    void Update()
    {
        Material mat = targetMaterial;
        if (mat == null && meshRenderer != null)
            mat = meshRenderer.sharedMaterial;
        if (mat == null) return;

        if (brdfLUT != null)
            mat.SetTexture("_BrdfLUT", brdfLUT);

        if (envCubemap != null)
            mat.SetTexture("_EnvCube", envCubemap);

        mat.SetFloat("_Roughness", roughness);
        mat.SetFloat("_Exposure", exposure);
        mat.SetFloat("_MaxSampleValue", maxSampleValue);

        if (pointLight != null)
        {
            mat.SetVector("_LightPos", pointLight.transform.position);
            mat.SetColor("_LightColor", pointLight.color);
            mat.SetFloat("_LightIntensity", pointLight.intensity);
        }
        else
        {
            mat.SetVector("_LightPos", lightPosition);
            mat.SetColor("_LightColor", lightColor);
            mat.SetFloat("_LightIntensity", lightIntensity);
        }
    }

    void OnDisable()
    {
        if (brdfLUT != null && brdfBinaryFile != null)
        {
            if (Application.isPlaying)
                Destroy(brdfLUT);
            else
                DestroyImmediate(brdfLUT);
            brdfLUT = null;
        }
    }
}
