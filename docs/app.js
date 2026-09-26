import * as THREE from "https://cdn.jsdelivr.net/npm/three@0.166.1/build/three.module.js";
import { OrbitControls } from "https://cdn.jsdelivr.net/npm/three@0.166.1/examples/jsm/controls/OrbitControls.js";

const state = {
  meshes: [],
  selectedMeshIndex: -1,
  uploadAbortController: null,
  uploadInProgress: false,
  lightIntensity: 50,
};

const dom = {
  panel: document.getElementById("panel"),
  openObjBtn: document.getElementById("openObjBtn"),
  objInput: document.getElementById("objInput"),
  assetList: document.getElementById("assetList"),
  lightIntensity: document.getElementById("lightIntensity"),
  lightIntensityValue: document.getElementById("lightIntensityValue"),
  toggleRotateBtn: document.getElementById("toggleRotateBtn"),
  uploadOverlay: document.getElementById("uploadOverlay"),
  uploadStage: document.getElementById("uploadStage"),
  uploadProgress: document.getElementById("uploadProgress"),
  cancelUploadBtn: document.getElementById("cancelUploadBtn"),
  canvas: document.getElementById("view"),
};

const renderer = new THREE.WebGLRenderer({ canvas: dom.canvas, antialias: true });
renderer.setPixelRatio(window.devicePixelRatio);
renderer.setSize(dom.canvas.clientWidth, dom.canvas.clientHeight, false);

const scene = new THREE.Scene();
scene.background = new THREE.Color(0xe8edf2);

const camera = new THREE.PerspectiveCamera(45, dom.canvas.clientWidth / dom.canvas.clientHeight, 0.1, 200);
camera.position.set(4.5, 2.5, 5.5);

const controls = new OrbitControls(camera, dom.canvas);
controls.enableDamping = true;
controls.autoRotate = true;
controls.autoRotateSpeed = 1.1;

const hemi = new THREE.HemisphereLight(0xffffff, 0x607080, 0.45);
scene.add(hemi);

const dirLight = new THREE.DirectionalLight(0xffffff, state.lightIntensity * 0.02);
dirLight.position.set(3, 4, 2);
scene.add(dirLight);

const root = new THREE.Group();
scene.add(root);

function setUploadUI(active) {
  state.uploadInProgress = active;
  dom.uploadOverlay.classList.toggle("hidden", !active);
  dom.uploadOverlay.setAttribute("aria-hidden", String(!active));
  dom.openObjBtn.disabled = active;
  dom.lightIntensity.disabled = active;
  dom.toggleRotateBtn.disabled = active;
}

function setUploadStage(text) {
  dom.uploadStage.textContent = text;
}

function setUploadProgress(v) {
  dom.uploadProgress.value = Math.max(0, Math.min(1, v));
}

function ensureAtLeastOneMesh() {
  if (state.meshes.length === 0) {
    const geo = new THREE.BoxGeometry(1, 1, 1);
    const mat = new THREE.MeshStandardMaterial({ color: 0xa3b3c2, metalness: 0.15, roughness: 0.65 });
    const mesh = new THREE.Mesh(geo, mat);
    root.add(mesh);
    state.meshes.push({
      name: "default_box",
      object3d: mesh,
      geometry: geo,
      material: mat,
    });
    state.selectedMeshIndex = 0;
    renderAssetList();
  }
}

function selectMesh(index) {
  if (index < 0 || index >= state.meshes.length) return;
  state.selectedMeshIndex = index;
  state.meshes.forEach((item, i) => {
    item.object3d.visible = i === index;
  });
  renderAssetList();
}

function removeMesh(index) {
  if (state.meshes.length <= 1) return;
  if (index < 0 || index >= state.meshes.length) return;

  const [removed] = state.meshes.splice(index, 1);
  root.remove(removed.object3d);
  removed.geometry.dispose();
  removed.material.dispose();

  if (state.selectedMeshIndex >= state.meshes.length) {
    state.selectedMeshIndex = state.meshes.length - 1;
  }
  if (index <= state.selectedMeshIndex) {
    state.selectedMeshIndex = Math.max(0, state.selectedMeshIndex);
  }
  selectMesh(state.selectedMeshIndex);
}

function renderAssetList() {
  dom.assetList.innerHTML = "";
  state.meshes.forEach((item, index) => {
    const li = document.createElement("li");
    li.className = "asset-item" + (index === state.selectedMeshIndex ? " active" : "");

    const selectBtn = document.createElement("button");
    selectBtn.className = "asset-select";
    selectBtn.textContent = item.name;
    selectBtn.addEventListener("click", () => selectMesh(index));

    const delBtn = document.createElement("button");
    delBtn.className = "asset-delete";
    delBtn.textContent = "x";
    delBtn.disabled = state.meshes.length <= 1;
    delBtn.addEventListener("click", (e) => {
      e.stopPropagation();
      removeMesh(index);
    });

    li.append(selectBtn, delBtn);
    dom.assetList.appendChild(li);
  });
}

function resolveIndex(rawIndex, length) {
  const idx = Number(rawIndex);
  if (Number.isNaN(idx)) return null;
  return idx >= 0 ? idx - 1 : length + idx;
}

function parseFaceToken(token) {
  const parts = token.split("/");
  return {
    v: parts[0] ? Number(parts[0]) : null,
    vt: parts[1] ? Number(parts[1]) : null,
    vn: parts[2] ? Number(parts[2]) : null,
  };
}

async function parseOBJWithProgress(text, { signal, onProgress, onStage }) {
  const lines = text.split(/\r?\n/);
  const vertices = [];
  const texcoords = [];
  const normals = [];
  const faces = [];

  onStage("processing OBJ");
  onProgress(0);

  const chunkSize = 1200;
  for (let i = 0; i < lines.length; i++) {
    if (signal.aborted) throw new Error("Upload canceled");

    const line = lines[i].trim();
    if (line.length === 0 || line.startsWith("#")) continue;

    const parts = line.split(/\s+/);
    const type = parts[0];

    if (type === "v" && parts.length >= 4) {
      vertices.push([Number(parts[1]), Number(parts[2]), Number(parts[3])]);
    } else if (type === "vt" && parts.length >= 3) {
      texcoords.push([Number(parts[1]), Number(parts[2])]);
    } else if (type === "vn" && parts.length >= 4) {
      normals.push([Number(parts[1]), Number(parts[2]), Number(parts[3])]);
    } else if (type === "f" && parts.length >= 4) {
      const f = parts.slice(1).map(parseFaceToken);
      for (let k = 1; k < f.length - 1; k++) {
        faces.push([f[0], f[k], f[k + 1]]);
      }
    }

    if (i % chunkSize === 0) {
      onProgress((i / Math.max(1, lines.length)) * 0.7);
      await new Promise((r) => setTimeout(r, 0));
    }
  }

  const hasVT = texcoords.length > 0 && faces.some((tri) => tri.every((c) => c.vt !== null));

  const positions = [];
  const outNormals = [];
  const outUVs = [];

  for (let i = 0; i < faces.length; i++) {
    if (signal.aborted) throw new Error("Upload canceled");
    const tri = faces[i];

    for (let j = 0; j < 3; j++) {
      const c = tri[j];
      const vi = resolveIndex(c.v, vertices.length);
      if (vi === null || vi < 0 || vi >= vertices.length) continue;
      const v = vertices[vi];
      positions.push(v[0], v[1], v[2]);

      if (c.vn !== null) {
        const ni = resolveIndex(c.vn, normals.length);
        if (ni !== null && ni >= 0 && ni < normals.length) {
          const n = normals[ni];
          outNormals.push(n[0], n[1], n[2]);
        } else {
          outNormals.push(0, 0, 0);
        }
      } else {
        outNormals.push(0, 0, 0);
      }

      if (hasVT && c.vt !== null) {
        const ti = resolveIndex(c.vt, texcoords.length);
        if (ti !== null && ti >= 0 && ti < texcoords.length) {
          const t = texcoords[ti];
          outUVs.push(t[0], t[1]);
        } else {
          outUVs.push(0, 0);
        }
      }
    }

    if (i % 1600 === 0) {
      onProgress(0.7 + (i / Math.max(1, faces.length)) * 0.2);
      await new Promise((r) => setTimeout(r, 0));
    }
  }

  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute("position", new THREE.Float32BufferAttribute(positions, 3));

  if (outNormals.some((n) => n !== 0)) {
    geometry.setAttribute("normal", new THREE.Float32BufferAttribute(outNormals, 3));
  } else {
    geometry.computeVertexNormals();
  }

  if (hasVT && outUVs.length === (positions.length / 3) * 2) {
    geometry.setAttribute("uv", new THREE.Float32BufferAttribute(outUVs, 2));
  } else {
    onStage("creating VT mapping");
    generateBoxProjectionUV(geometry, { signal, onProgress });
  }

  geometry.computeBoundingBox();
  const box = geometry.boundingBox;
  const size = new THREE.Vector3();
  box.getSize(size);
  const maxAxis = Math.max(size.x, size.y, size.z) || 1;
  const center = new THREE.Vector3();
  box.getCenter(center);

  geometry.translate(-center.x, -center.y, -center.z);
  geometry.scale(1 / maxAxis, 1 / maxAxis, 1 / maxAxis);

  onProgress(1);
  return geometry;
}

function generateBoxProjectionUV(geometry, { signal, onProgress }) {
  const pos = geometry.getAttribute("position");
  const normal = geometry.getAttribute("normal");

  if (!normal) {
    geometry.computeVertexNormals();
  }
  const nrm = geometry.getAttribute("normal");

  geometry.computeBoundingBox();
  const box = geometry.boundingBox;
  const min = box.min;
  const max = box.max;

  const rx = Math.max(1e-6, max.x - min.x);
  const ry = Math.max(1e-6, max.y - min.y);
  const rz = Math.max(1e-6, max.z - min.z);

  const uv = new Float32Array(pos.count * 2);
  for (let i = 0; i < pos.count; i++) {
    if (signal.aborted) throw new Error("Upload canceled");

    const x = pos.getX(i);
    const y = pos.getY(i);
    const z = pos.getZ(i);
    const nx = Math.abs(nrm.getX(i));
    const ny = Math.abs(nrm.getY(i));
    const nz = Math.abs(nrm.getZ(i));

    let u = 0;
    let v = 0;
    if (nx >= ny && nx >= nz) {
      u = (z - min.z) / rz;
      v = (y - min.y) / ry;
    } else if (ny >= nx && ny >= nz) {
      u = (x - min.x) / rx;
      v = (z - min.z) / rz;
    } else {
      u = (x - min.x) / rx;
      v = (y - min.y) / ry;
    }

    uv[i * 2] = u;
    uv[i * 2 + 1] = v;

    if (i % 4000 === 0) {
      onProgress(0.7 + (i / Math.max(1, pos.count)) * 0.25);
    }
  }

  geometry.setAttribute("uv", new THREE.Float32BufferAttribute(uv, 2));
}

async function addOBJFile(file) {
  if (!file) return;
  const readerText = await file.text();

  const controller = new AbortController();
  state.uploadAbortController = controller;

  setUploadUI(true);
  setUploadStage("processing OBJ");
  setUploadProgress(0);

  try {
    const geometry = await parseOBJWithProgress(readerText, {
      signal: controller.signal,
      onProgress: setUploadProgress,
      onStage: setUploadStage,
    });

    const material = new THREE.MeshStandardMaterial({
      color: 0xc6d2dd,
      metalness: 0.2,
      roughness: 0.55,
    });

    const mesh = new THREE.Mesh(geometry, material);
    state.meshes.forEach((m) => {
      m.object3d.visible = false;
    });

    root.add(mesh);
    state.meshes.push({
      name: file.name,
      object3d: mesh,
      geometry,
      material,
    });

    selectMesh(state.meshes.length - 1);
  } catch (err) {
    if (err && err.message !== "Upload canceled") {
      console.error(err);
      alert("Failed to parse OBJ file.");
    }
  } finally {
    state.uploadAbortController = null;
    setUploadUI(false);
    setUploadProgress(0);
    setUploadStage("processing OBJ");
  }
}

function bindEvents() {
  dom.openObjBtn.addEventListener("click", () => {
    if (state.uploadInProgress) return;
    dom.objInput.value = "";
    dom.objInput.click();
  });

  dom.objInput.addEventListener("change", async (e) => {
    const file = e.target.files?.[0];
    await addOBJFile(file);
  });

  dom.cancelUploadBtn.addEventListener("click", () => {
    state.uploadAbortController?.abort();
  });

  dom.lightIntensity.addEventListener("input", (e) => {
    state.lightIntensity = Number(e.target.value);
    dom.lightIntensityValue.textContent = String(state.lightIntensity);
    dirLight.intensity = state.lightIntensity * 0.02;
  });

  dom.toggleRotateBtn.addEventListener("click", () => {
    controls.autoRotate = !controls.autoRotate;
  });

  window.addEventListener("resize", () => {
    const w = dom.canvas.clientWidth;
    const h = dom.canvas.clientHeight;
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
    renderer.setSize(w, h, false);
  });
}

function animate() {
  controls.update();
  renderer.render(scene, camera);
  requestAnimationFrame(animate);
}

ensureAtLeastOneMesh();
bindEvents();
animate();
