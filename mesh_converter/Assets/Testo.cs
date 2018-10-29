using UnityEngine;

public class Testo : MonoBehaviour 
{
    [SerializeField] float yaw;
    [SerializeField] float pitch;

	void Update () 
    {
        var a = Quaternion.AngleAxis(yaw, Vector3.up);
        var pitchAxis = a * Vector3.right;
        var b = Quaternion.AngleAxis(pitch, pitchAxis);

        transform.rotation = b * a;

        Quaternion q = Quaternion.AngleAxis(90, Random.onUnitSphere);
        var v = Random.onUnitSphere;
        var v1 = q * v;

        Debug.Log(string.Format("({0} {1} {2} {3}) * ({4} {5} {6}) = ({7} {8} {9})",
            q.x, q.y, q.z, q.w,
            v.x, v.y, v.z,
            v1.x, v1.y, v1.z));

        // https://answers.unity.com/questions/372371/multiply-quaternion-by-vector3-how-is-done.html
        // https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm
	}
}
