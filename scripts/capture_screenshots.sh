#!/usr/bin/env bash
set -euo pipefail

# Capture screenshots of the mesh-viewer window on macOS.
# Usage:
#   ./scripts/capture_screenshots.sh
#   ./scripts/capture_screenshots.sh --count 4 --interval 2 --output-dir docs/screenshots
#   ./scripts/capture_screenshots.sh --no-launch

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$PROJECT_ROOT/docs/screenshots"
COUNT=3
INTERVAL=2
LAUNCH_IF_NEEDED=1
MANUAL_MODE=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --output-dir)
      OUTPUT_DIR="$2"
      shift 2
      ;;
    --count)
      COUNT="$2"
      shift 2
      ;;
    --interval)
      INTERVAL="$2"
      shift 2
      ;;
    --no-launch)
      LAUNCH_IF_NEEDED=0
      shift
      ;;
    --manual)
      MANUAL_MODE=1
      shift
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done

# Keep output anchored to project root unless an absolute path is provided.
if [[ "$OUTPUT_DIR" != /* ]]; then
  OUTPUT_DIR="$PROJECT_ROOT/$OUTPUT_DIR"
fi

mkdir -p "$OUTPUT_DIR"
echo "Output directory: $OUTPUT_DIR"

launched_pid=""
if ! pgrep -x "mesh-viewer" >/dev/null 2>&1; then
  if [[ "$LAUNCH_IF_NEEDED" -eq 0 ]]; then
    echo "mesh-viewer is not running. Start it first or remove --no-launch."
    exit 1
  fi

  if [[ ! -x "./build/mesh-viewer" ]]; then
    echo "Executable ./build/mesh-viewer not found. Build first with: cmake --build build -j4"
    exit 1
  fi

  ./build/mesh-viewer >/dev/null 2>&1 &
  launched_pid="$!"
  echo "Started mesh-viewer (pid=$launched_pid)."
fi

get_bounds() {
  local out
  out="$(osascript <<'APPLESCRIPT' 2>&1
  tell application "System Events"
    if not (exists process "mesh-viewer") then
      return ""
    end if
    tell process "mesh-viewer"
      if (count of windows) is 0 then
        return ""
      end if
      set p to position of window 1
      set s to size of window 1
      return (item 1 of p as text) & "," & (item 2 of p as text) & "," & (item 1 of s as text) & "," & (item 2 of s as text)
    end tell
  end tell
APPLESCRIPT
  )"

  if [[ $? -ne 0 ]]; then
    if [[ "$out" == *"not allowed assistive access"* ]]; then
      echo "__ACCESS_DENIED__"
      return 0
    fi
    echo ""
    return 0
  fi

  echo "$out"
}

capture_manual() {
  echo "Accessibility permission is unavailable. Switching to manual window capture mode."
  echo "Press 'c' to enable one screenshot capture, or 'q' to quit."
  echo "Crosshair appears only after pressing 'c'."

  for i in $(seq 1 "$COUNT"); do
    while true; do
      printf "[%02d/%02d] Press c to capture, q to quit: " "$i" "$COUNT"
      IFS= read -r -n1 key
      echo
      if [[ "$key" == "c" || "$key" == "C" ]]; then
        break
      fi
      if [[ "$key" == "q" || "$key" == "Q" ]]; then
        echo "Stopped by user."
        return 0
      fi
    done

    filename=$(printf "%s/%02d-view.png" "$OUTPUT_DIR" "$i")
    set +e
    screencapture -x -i -W "$filename"
    status=$?
    set -e
    if [[ "$status" -ne 0 ]]; then
      echo "Capture canceled or failed for $filename (exit=$status)."
      continue
    fi
    if [[ ! -s "$filename" ]]; then
      echo "Capture produced no file: $filename"
      continue
    fi
    echo "Saved $filename"
    if [[ "$i" -lt "$COUNT" ]]; then
      sleep "$INTERVAL"
    fi
  done
}

if [[ "$MANUAL_MODE" -eq 1 ]]; then
  capture_manual
  if [[ -n "$launched_pid" ]]; then
    kill "$launched_pid" >/dev/null 2>&1 || true
  fi
  echo "Done."
  exit 0
fi

bounds=""
for _ in {1..20}; do
  bounds="$(get_bounds | tr -d ' ')"
  if [[ "$bounds" == "__ACCESS_DENIED__" ]]; then
    capture_manual
    if [[ -n "$launched_pid" ]]; then
      kill "$launched_pid" >/dev/null 2>&1 || true
    fi
    echo "Done."
    exit 0
  fi
  if [[ -n "$bounds" ]]; then
    break
  fi
  sleep 0.5
done

if [[ -z "$bounds" ]]; then
  echo "Failed to locate mesh-viewer window bounds."
  echo "On first run, grant Accessibility permission to Terminal/iTerm in System Settings > Privacy & Security > Accessibility."
  [[ -n "$launched_pid" ]] && kill "$launched_pid" >/dev/null 2>&1 || true
  exit 1
fi

IFS=',' read -r x y w h <<< "$bounds"
if [[ "$w" -le 0 || "$h" -le 0 ]]; then
  echo "Invalid capture bounds: $bounds"
  [[ -n "$launched_pid" ]] && kill "$launched_pid" >/dev/null 2>&1 || true
  exit 1
fi

echo "Capturing $COUNT screenshot(s) from bounds: $x,$y,$w,$h"
for i in $(seq 1 "$COUNT"); do
  filename=$(printf "%s/%02d-view.png" "$OUTPUT_DIR" "$i")
  set +e
  screencapture -x -R"$x,$y,$w,$h" "$filename"
  status=$?
  set -e
  if [[ "$status" -ne 0 ]]; then
    echo "Capture failed for $filename (exit=$status)."
    continue
  fi
  if [[ ! -s "$filename" ]]; then
    echo "Capture produced no file: $filename"
    continue
  fi
  echo "Saved $filename"
  if [[ "$i" -lt "$COUNT" ]]; then
    sleep "$INTERVAL"
  fi
done

if [[ -n "$launched_pid" ]]; then
  kill "$launched_pid" >/dev/null 2>&1 || true
fi

echo "Done."
