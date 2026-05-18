using UnityEngine;
using System.IO;

public static class MerlBrdfLoader
{
    public const int ThetaH = 90;
    public const int ThetaD = 90;
    public const int PhiD = 180;

    const double RedScale = 1.0 / 1500.0;
    const double GreenScale = 1.15 / 1500.0;
    const double BlueScale = 1.66 / 1500.0;

    public static double[] LoadRawDoubles(string path)
    {
        byte[] bytes = File.ReadAllBytes(path);
        int count = bytes.Length / 8;
        double[] data = new double[count];
        for (int i = 0; i < count; i++)
            data[i] = System.BitConverter.ToDouble(bytes, i * 8);
        return data;
    }

    public static Texture3D LoadBrdfTexture(string binaryPath)
    {
        double[] raw = LoadRawDoubles(binaryPath);

        // Verify header: 3 ints stored as doubles (or read as ints from raw bytes)
        // Actually the header in the file is 3 ints (4 bytes each), not doubles.
        // We read the raw bytes and parse properly.
        byte[] bytes = File.ReadAllBytes(binaryPath);

        int thetaH = System.BitConverter.ToInt32(bytes, 0);
        int thetaD = System.BitConverter.ToInt32(bytes, 4);
        int phiD = System.BitConverter.ToInt32(bytes, 8);

        if (thetaH != ThetaH || thetaD != ThetaD || phiD != PhiD)
        {
            Debug.LogWarning($"BRDF dimensions mismatch. Expected [{ThetaH},{ThetaD},{PhiD}], got [{thetaH},{thetaD},{phiD}]");
        }

        int N = thetaH * thetaD * phiD;

        // Read doubles after the 12-byte header
        double[] brdf = new double[3 * N];
        for (int i = 0; i < 3 * N; i++)
            brdf[i] = System.BitConverter.ToDouble(bytes, 12 + i * 8);

        // Texture3D: phiD x thetaD x thetaH
        Texture3D tex = new Texture3D(phiD, thetaD, thetaH, TextureFormat.RGBAHalf, false);
        tex.wrapMode = TextureWrapMode.Clamp;
        tex.filterMode = FilterMode.Bilinear;

        Color[] pixels = new Color[phiD * thetaD * thetaH];

        for (int th = 0; th < thetaH; th++)
        {
            for (int td = 0; td < thetaD; td++)
            {
                for (int pd = 0; pd < phiD; pd++)
                {
                    int idx = pd + td * phiD + th * phiD * thetaD;

                    float r = Mathf.Max(0f, (float)(brdf[idx] * RedScale));
                    float g = Mathf.Max(0f, (float)(brdf[idx + N] * GreenScale));
                    float b = Mathf.Max(0f, (float)(brdf[idx + 2 * N] * BlueScale));

                    // Texture3D pixel layout: x=pd, y=td, z=th
                    int pixelIdx = pd + td * phiD + th * phiD * thetaD;
                    pixels[pixelIdx] = new Color(r, g, b, 1.0f);
                }
            }
        }

        tex.SetPixels(pixels);
        tex.Apply();
        return tex;
    }

    public static Texture3D LoadBrdfTextureFromRawData(byte[] binaryData)
    {
        int thetaH = System.BitConverter.ToInt32(binaryData, 0);
        int thetaD = System.BitConverter.ToInt32(binaryData, 4);
        int phiD = System.BitConverter.ToInt32(binaryData, 8);

        if (thetaH != ThetaH || thetaD != ThetaD || phiD != PhiD)
        {
            Debug.LogWarning($"BRDF dimensions mismatch. Expected [{ThetaH},{ThetaD},{PhiD}], got [{thetaH},{thetaD},{phiD}]");
        }

        int N = thetaH * thetaD * phiD;
        double[] brdf = new double[3 * N];
        for (int i = 0; i < 3 * N; i++)
            brdf[i] = System.BitConverter.ToDouble(binaryData, 12 + i * 8);

        Texture3D tex = new Texture3D(phiD, thetaD, thetaH, TextureFormat.RGBAHalf, false);
        tex.wrapMode = TextureWrapMode.Clamp;
        tex.filterMode = FilterMode.Bilinear;

        Color[] pixels = new Color[phiD * thetaD * thetaH];

        for (int th = 0; th < thetaH; th++)
        {
            for (int td = 0; td < thetaD; td++)
            {
                for (int pd = 0; pd < phiD; pd++)
                {
                    int idx = pd + td * phiD + th * phiD * thetaD;
                    float r = Mathf.Max(0f, (float)(brdf[idx] * RedScale));
                    float g = Mathf.Max(0f, (float)(brdf[idx + N] * GreenScale));
                    float b = Mathf.Max(0f, (float)(brdf[idx + 2 * N] * BlueScale));
                    int pixelIdx = pd + td * phiD + th * phiD * thetaD;
                    pixels[pixelIdx] = new Color(r, g, b, 1.0f);
                }
            }
        }

        tex.SetPixels(pixels);
        tex.Apply();
        return tex;
    }
}
