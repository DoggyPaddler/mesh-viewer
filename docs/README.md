# Mesh Viewer Web (docs)

## Run locally
Use any static server from the project root.

Option 1 (Node):

npx serve docs

Option 2 (VS Code Live Server):
Open docs/index.html with Live Server.

Then open the printed URL in browser.

## Current features
- Upload OBJ file in browser.
- Upload progress overlay with stage text:
  - processing OBJ
  - creating VT mapping (auto box projection if OBJ has no VT)
- Cancel upload while parsing.
- Asset list with right-side x button to delete loaded mesh.
- Light Intensity control (1..100), mapped with scale factor 0.02.
- Orbit camera with Pause/Resume auto-rotation.

## Notes
- This web version is a JS prototype under docs (no native C++/WASM build yet).
- It is designed to mirror key interactions from the desktop viewer.
- If you want, next step is to replace JS OBJ processing with C++ core compiled to WebAssembly.
