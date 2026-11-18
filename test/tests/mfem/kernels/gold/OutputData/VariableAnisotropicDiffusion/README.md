# Variable Anisotropic Diffusion Gold

You can regenerate the data in `Run0/` using MFEM revision a7dbea190,
after applying the below patch to the `examples/ex29p.cpp` file.
The patch updates the example to write the output mesh in PVD ASCII format.
Run the example with no extra arguments.

```patch
diff --git a/examples/ex29p.cpp b/examples/ex29p.cpp
index 2b2251babf..b67360dce2 100644
--- a/examples/ex29p.cpp
+++ b/examples/ex29p.cpp
@@ -235,6 +235,21 @@ int main(int argc, char *argv[])
       flux.Save(flux_ofs);
    }

+   pmesh.Save("ex29p.mesh"); // mesh used as input to test
+   {
+     auto pd = std::make_unique<ParaViewDataCollection>("Run0", &pmesh);
+     pd->SetPrefixPath("ParaView");
+     pd->RegisterField("u", &x);
+     pd->SetLevelsOfDetail(order);
+     pd->SetDataFormat(VTKFormat::ASCII);
+     pd->SetHighOrderOutput(true);
+     pd->SetPrecision(9);
+     pd->SetLevelsOfDetail(1);
+     pd->SetTime(1);
+     pd->SetCycle(1);
+     pd->Save();
+   }
+
    // 17. Send the solution by socket to a GLVis server.
    if (visualization)
    {
```
