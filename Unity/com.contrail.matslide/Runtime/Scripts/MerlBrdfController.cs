using UnityEngine;

[ExecuteAlways]
public class MerlBrdfController : MonoBehaviour
{
    [Header("BRDF Data")]
    public Texture3D brdfLUT;
    public TextAsset brdfBinaryFile;

    [Header("Scene Light")]
    public Light targetLight;

    [Header("Lighting (fallback)")]
    public Color lightColor = Color.white;
    public float lightIntensity = 100f;
    public Vector3 lightPosition = new Vector3(2f, 3f, 1f);

    [Header("Light Rotation (fallback)")]
    public bool rotateLight = true;
    public float rotationSpeed = 0.1f;
    public float rotationRadius = 3f;
    public float lightHeight = 3f;

    [Header("References")]
    public Material targetMaterial;
    public MeshRenderer meshRenderer;

    float currentAngle;

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
        if (targetLight == null && rotateLight)
        {
            currentAngle += rotationSpeed * Time.deltaTime;
            lightPosition.x = Mathf.Cos(currentAngle) * rotationRadius;
            lightPosition.z = Mathf.Sin(currentAngle) * rotationRadius;
            lightPosition.y = lightHeight;
        }

        ApplyToMaterial();
    }

    void ApplyToMaterial()
    {
        Material mat = targetMaterial;
        if (mat == null && meshRenderer != null)
            mat = meshRenderer.sharedMaterial;
        if (mat == null) return;

        if (brdfLUT != null)
            mat.SetTexture("_BrdfLUT", brdfLUT);

        if (targetLight != null)
        {
            mat.SetVector("_LightPos", targetLight.transform.position);
            mat.SetColor("_LightColor", targetLight.color);
            mat.SetFloat("_LightIntensity", targetLight.intensity);
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
