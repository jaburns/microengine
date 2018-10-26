using System;
using UnityEngine;
using System.Collections.Generic;
using System.IO;

[ExecuteInEditMode]
public class MeshExporter : MonoBehaviour 
{
    [SerializeField] bool export;

    static byte[] GetBytes<T>(Func<T,byte[]> writer, params T[] ts)
    {
        var output = new List<byte>();

        foreach (var t in ts) 
            output.AddRange(writer(t));

        return output.ToArray();
    }

    static void WriteBytes<T>(List<byte> output, Func<T,byte[]> writer, IEnumerable<T> ts)
    {
        foreach (var t in ts) 
            output.AddRange(writer(t));
    }

    static ushort GetMeshFlags(Mesh mesh)
    {
        // TODO write if we're including normals, tangents, vert colors etc.
        return 0;
    }

    static byte[] Serialize(Mesh mesh, Material[] mat)
    {
        var file = new List<byte>();

        file.AddRange(BitConverter.GetBytes((ushort)mesh.vertexCount));
        file.AddRange(BitConverter.GetBytes(GetMeshFlags(mesh)));
        WriteBytes(file, v => GetBytes(BitConverter.GetBytes, v.x, v.y, v.z), mesh.vertices);
        WriteBytes(file, v => GetBytes(BitConverter.GetBytes, v.x, v.y, v.z), mesh.normals);
        WriteBytes(file, v => GetBytes(BitConverter.GetBytes, v.x, v.y),      mesh.uv);

        file.AddRange(BitConverter.GetBytes((ushort)mesh.subMeshCount));

        Debug.Log("Submesh count: " + mesh.subMeshCount);

        for (int i = 0; i < mesh.subMeshCount; ++i)
        {
            var tris = mesh.GetTriangles(i);
            file.AddRange(BitConverter.GetBytes((ushort)tris.Length));
            WriteBytes(file, t => BitConverter.GetBytes((ushort)t), tris);
        }

        return file.ToArray();
    }

    void Update()
    {
        if (export)
        {
            export = false;

            var exportMesh = GetComponent<MeshFilter>().sharedMesh;
            var exportMaterials = GetComponent<MeshRenderer>().sharedMaterials;

            File.WriteAllBytes(Application.dataPath + "/mesh.umesh", Serialize(exportMesh, exportMaterials)); 
            Debug.Log("Wrote umesh file");
        }
    }
}
