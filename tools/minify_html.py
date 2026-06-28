"""
PlatformIO advanced script for minifying LittleFS HTML assets.

This script hooks PlatformIO's `buildfs` action and runs immediately before the
LittleFS filesystem image is generated. It copies editable source files from
`web/` into PlatformIO's generated LittleFS `data/` directory, then minifies
every `.html` and `.htm` file in `data/`, writes a gzip-compressed copy, and
removes the uncompressed HTML so only the `.gz` asset is included in LittleFS.

Dependency:
  Install `minify-html` into PlatformIO's Python environment:

    C:\\Users\\Ryan\\.platformio\\penv\\Scripts\\python.exe -m pip install minify-html
"""

from pathlib import Path
import gzip
import importlib
import shutil
import sys

Import("env")

try:
    script_dir = str(Path(env.subst("$PROJECT_DIR")) / "tools")
    if script_dir in sys.path:
        sys.path.remove(script_dir)

    minify_html = importlib.import_module("minify_html")
except ImportError as error:
    raise ImportError(
        "Missing Python package 'minify-html'. Install it into PlatformIO's Python "
        "environment with: "
        "C:\\Users\\Ryan\\.platformio\\penv\\Scripts\\python.exe -m pip install minify-html"
    ) from error


def minify_littlefs_html(target, source, env):
    """Copy web sources, then minify and gzip HTML before LittleFS image creation."""
    project_dir = Path(env.subst("$PROJECT_DIR"))
    web_dir = project_dir / "web"
    data_dir = project_dir / "data"

    if not web_dir.exists():
        print("[minify-html] web/ directory not found; skipping web asset generation.")
        return

    data_dir.mkdir(exist_ok=True)

    for existing in data_dir.iterdir():
        if existing.is_dir():
            shutil.rmtree(existing)
        else:
            existing.unlink()

    for source_file in web_dir.rglob("*"):
        if not source_file.is_file():
            continue

        relative_file = source_file.relative_to(web_dir)
        target_file = data_dir / relative_file
        target_file.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_file, target_file)

    print(f"[minify-html] Copied web assets from {web_dir.relative_to(project_dir)} to {data_dir.relative_to(project_dir)}.")

    html_files = sorted(
        path
        for path in data_dir.rglob("*")
        if path.is_file() and path.suffix.lower() in (".html", ".htm")
    )

    if not html_files:
        print("[minify-html] No .html or .htm files found in data/.")
        return

    for html_file in html_files:
        original_html = html_file.read_text(encoding="utf-8")

        minified_html = minify_html.minify(
            original_html,
            keep_comments=False,
            minify_css=True,
            minify_js=True,
            minify_doctype=True,
        ).strip()

        html_file.write_text(minified_html, encoding="utf-8")
        gzip_file = html_file.with_name(f"{html_file.name}.gz")

        with gzip.GzipFile(filename="", mode="wb", fileobj=gzip_file.open("wb"), mtime=0) as archive:
            archive.write(minified_html.encode("utf-8"))

        original_bytes = len(original_html.encode("utf-8"))
        minified_bytes = len(minified_html.encode("utf-8"))
        gzip_bytes = gzip_file.stat().st_size
        saved_bytes = original_bytes - minified_bytes
        gzip_saved_bytes = original_bytes - gzip_bytes
        relative_path = html_file.relative_to(project_dir)
        relative_gzip_path = gzip_file.relative_to(project_dir)
        html_file.unlink()

        print(
            "[minify-html] Minified "
            f"{relative_path} "
            f"({original_bytes} -> {minified_bytes} bytes, saved {saved_bytes} bytes)"
        )
        print(
            "[minify-html] Gzipped  "
            f"{relative_gzip_path} "
            f"({original_bytes} -> {gzip_bytes} bytes, saved {gzip_saved_bytes} bytes)"
        )
        print(f"[minify-html] Removed  {relative_path}; keeping gzip asset only.")


env.AddPreAction("buildfs", minify_littlefs_html)
