#!/usr/bin/env python3
"""Generate a browsable, static index page for every directory under files/."""

from __future__ import annotations

import argparse
import html
import json
import mimetypes
import os
from datetime import datetime
from pathlib import Path
from urllib.parse import quote


INDEX_NAME = "index.html"
METADATA_NAME = "metadata.json"


def format_size(size: int) -> str:
    if size < 1024:
        return f"{size} B"
    units = ("KB", "MB", "GB", "TB")
    value = float(size)
    for unit in units:
        value /= 1024
        if value < 1024 or unit == units[-1]:
            return f"{value:.1f} {unit}"
    return f"{size} B"


def icon_for(name: str, is_dir: bool) -> str:
    if is_dir:
        return "folder"
    kind = mimetypes.guess_type(name)[0] or ""
    if kind.startswith("image/"):
        return "image"
    if kind.startswith("video/"):
        return "video"
    if kind.startswith("audio/"):
        return "audio"
    if name.lower().endswith((".zip", ".gz", ".rar", ".7z", ".tar")):
        return "archive"
    return "file"


def svg_icon(kind: str) -> str:
    paths = {
        "folder": '<path d="M3 6.5A1.5 1.5 0 0 1 4.5 5h4l1.7 2H19.5A1.5 1.5 0 0 1 21 8.5v8A1.5 1.5 0 0 1 19.5 18h-15A1.5 1.5 0 0 1 3 16.5z"/>',
        "image": '<rect x="3" y="4" width="18" height="16" rx="2"/><circle cx="8.5" cy="9" r="1.5"/><path d="m4 17 5-5 3 3 2-2 6 6"/>',
        "video": '<rect x="3" y="5" width="14" height="14" rx="2"/><path d="m17 10 4-2v8l-4-2z"/>',
        "audio": '<path d="M9 18V6l10-2v12"/><circle cx="6" cy="18" r="3"/><circle cx="16" cy="16" r="3"/>',
        "archive": '<path d="M4 5h16v4H4zM6 9v10h12V9M9 13h6"/>',
        "file": '<path d="M6 3h8l4 4v14H6z"/><path d="M14 3v5h5M9 13h6M9 17h6"/>',
    }
    return f'<svg viewBox="0 0 24 24" aria-hidden="true">{paths[kind]}</svg>'


def directory_size(path: Path) -> int:
    return sum(
        file.stat().st_size
        for file in path.rglob("*")
        if file.is_file() and file.name != INDEX_NAME
    )


def relative_url(path: Path, current: Path) -> str:
    return quote(os.path.relpath(path, current).replace(os.sep, "/"), safe="/")


def page_html(
    directory: Path,
    root: Path,
    entries: list[tuple[Path, bool, int, float, str]],
) -> str:
    rel_dir = directory.relative_to(root)
    title = "黑青菜的文件目录" if not rel_dir.parts else rel_dir.name
    browser_title = title if not rel_dir.parts else f"{title} · 黑青菜的文件目录"
    root_href = "../" * len(rel_dir.parts) or "./"
    crumbs = [f'<a href="{root_href}">全部文件</a>']
    for index, part in enumerate(rel_dir.parts):
        href = "../" * (len(rel_dir.parts) - index - 1)
        crumbs.append(f'<span>/</span> <a href="{href}">{html.escape(part)}</a>')

    rows = []
    if rel_dir.parts:
        rows.append(
            '<tr class="directory" data-name=".." data-size="0" data-time="0">'
            '<td><a class="entry" href="../"><span class="icon folder">'
            f"{svg_icon('folder')}</span><span>返回上级目录</span></a></td>"
            '<td data-value="0">-</td><td data-value="0">-</td><td>目录</td><td>-</td></tr>'
        )
    for path, is_dir, size, modified, comment in entries:
        name = path.name
        href = relative_url(path / INDEX_NAME if is_dir else path, directory)
        kind = icon_for(name, is_dir)
        type_name = "目录" if is_dir else (mimetypes.guess_type(name)[0] or "文件").split("/")[-1]
        rows.append(
            f'<tr class="{"directory" if is_dir else "file"}" data-name="{html.escape(name.lower())}" '
            f'data-size="{size}" data-time="{modified:.3f}">'
            f'<td><a class="entry" href="{href}"><span class="icon {kind}">{svg_icon(kind)}</span>'
            f'<span>{html.escape(name)}</span></a></td>'
            f'<td data-value="{size}">{format_size(size) if is_dir or size else "0 B"}</td>'
            f'<td data-value="{modified}">{datetime.fromtimestamp(modified).strftime("%Y-%m-%d %H:%M")}</td>'
            f'<td>{html.escape(type_name)}</td><td class="comment">{html.escape(comment) or "-"}</td></tr>'
        )

    return f"""<!doctype html>
<html lang="zh-CN">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(browser_title)}</title>
<style>
:root{{color-scheme:light;--bg:#f5f7fb;--card:#fff;--text:#172033;--muted:#73809a;--line:#e9edf5;--accent:#5b63f5;--accent-soft:#eef0ff;--shadow:0 18px 50px #25345b12}}
*{{box-sizing:border-box}}body{{margin:0;background:var(--bg);color:var(--text);font:14px/1.5 Inter,ui-sans-serif,system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}}
.shell{{max-width:1060px;margin:0 auto;padding:52px 22px 70px}}.hero{{display:flex;align-items:flex-end;justify-content:space-between;gap:20px;margin-bottom:28px}}
h1{{font-size:clamp(26px,4vw,40px);letter-spacing:-.04em;margin:12px 0 6px}}.eyebrow{{color:var(--accent);font-weight:700;letter-spacing:.08em;text-transform:uppercase;font-size:12px}}
.crumbs{{display:flex;flex-wrap:wrap;gap:7px;color:var(--muted)}}a{{color:inherit;text-decoration:none}}.crumbs a:hover,.entry:hover{{color:var(--accent)}}
.search{{width:min(290px,100%);border:1px solid var(--line);border-radius:14px;padding:12px 15px;background:var(--card);outline:0;box-shadow:0 4px 14px #26345b08;font:inherit}}.search:focus{{border-color:var(--accent);box-shadow:0 0 0 4px var(--accent-soft)}}
.card{{background:var(--card);border:1px solid var(--line);border-radius:20px;box-shadow:var(--shadow);overflow:hidden}}table{{width:100%;border-collapse:collapse}}th,td{{padding:15px 20px;text-align:left;border-bottom:1px solid var(--line)}}th{{color:var(--muted);font-size:12px;font-weight:700;white-space:nowrap;background:#fbfcfe}}tbody tr:last-child td{{border-bottom:0}}tbody tr:hover{{background:#fafbff}}th button{{border:0;background:none;color:inherit;font:inherit;cursor:pointer;padding:0}}th button::after{{content:" ↕";opacity:.45}}th button.asc::after{{content:" ↑";opacity:1;color:var(--accent)}}th button.desc::after{{content:" ↓";opacity:1;color:var(--accent)}}
.entry{{display:flex;align-items:center;gap:12px;font-weight:600;min-width:230px}}.icon{{display:grid;place-items:center;width:34px;height:34px;border-radius:10px;flex:none;background:var(--accent-soft);color:var(--accent)}}.icon svg{{width:19px;height:19px;fill:none;stroke:currentColor;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round}}.icon.folder{{background:#fff4d9;color:#e6a516}}td:nth-child(2),td:nth-child(3){{color:var(--muted);white-space:nowrap;font-variant-numeric:tabular-nums}}td:nth-child(4){{color:var(--muted);text-transform:capitalize}}.empty{{padding:48px;text-align:center;color:var(--muted)}}@media(max-width:700px){{.shell{{padding:30px 12px}}.hero{{display:block}}.search{{margin-top:18px;width:100%}}th,td{{padding:12px 10px}}th:nth-child(3),td:nth-child(3),th:nth-child(4),td:nth-child(4){{display:none}}.entry{{min-width:0}}}}
</style>
</head>
<body><main class="shell"><div class="hero"><div><div class="eyebrow">HQC FILE HOST</div><h1>{html.escape(title)}</h1><nav class="crumbs">{" ".join(crumbs)}</nav></div><input class="search" type="search" placeholder="搜索当前目录…" aria-label="搜索当前目录"></div>
<section class="card"><table><thead><tr><th><button data-sort="name">名称</button></th><th><button data-sort="size">大小</button></th><th><button data-sort="time">修改时间</button></th><th><button data-sort="type">类型</button></th><th>说明</th></tr></thead><tbody>{"".join(rows) if rows else '<tr><td colspan="5" class="empty">这个目录是空的</td></tr>'}</tbody></table></section></main>
<script>
const body=document.querySelector("tbody"), search=document.querySelector(".search");
document.querySelectorAll("th button").forEach(button=>button.addEventListener("click",()=>{{const key=button.dataset.sort, asc=!button.classList.contains("asc");document.querySelectorAll("th button").forEach(b=>b.classList.remove("asc","desc"));button.classList.add(asc?"asc":"desc");const rows=[...body.querySelectorAll("tr")];rows.sort((a,b)=>{{let x,y;if(key==="name"){{x=a.dataset.name||"";y=b.dataset.name||""}}else if(key==="type"){{x=a.children[3]?.textContent||"";y=b.children[3]?.textContent||""}}else{{x=Number(a.dataset[key]||0);y=Number(b.dataset[key]||0)}}return (x<y?-1:x>y?1:0)*(asc?1:-1)}});rows.forEach(row=>body.appendChild(row))}}));
search.addEventListener("input",()=>{{const query=search.value.trim().toLowerCase();body.querySelectorAll("tr").forEach(row=>row.hidden=!!query&&!(row.textContent||"").toLowerCase().includes(query))}});
</script></body></html>"""


def generate(root: Path) -> None:
    root = root.resolve()
    if not root.is_dir():
        raise SystemExit(f"目录不存在: {root}")
    metadata_path = root.parent / METADATA_NAME
    metadata = {}
    if metadata_path.exists():
        try:
            metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as error:
            raise SystemExit(f"无法读取 {metadata_path}: {error}") from error
        if not isinstance(metadata, dict):
            raise SystemExit(f"{metadata_path} 必须是 JSON 对象")
    directories = [root] + sorted((p for p in root.rglob("*") if p.is_dir()), key=lambda p: str(p))
    for directory in directories:
        entries = []
        for path in sorted(directory.iterdir(), key=lambda p: (not p.is_dir(), p.name.lower())):
            if path.name in (INDEX_NAME, METADATA_NAME) or path.name.startswith("."):
                continue
            is_dir = path.is_dir()
            size = directory_size(path) if is_dir else path.stat().st_size
            key = path.relative_to(root).as_posix()
            value = metadata.get(key, {})
            comment = value.get("comment", "") if isinstance(value, dict) else value
            if not isinstance(comment, str):
                raise SystemExit(f"{metadata_path} 中 {key} 的 comment 必须是字符串")
            entries.append((path, is_dir, size, path.stat().st_mtime, comment))
        (directory / INDEX_NAME).write_text(page_html(directory, root, entries), encoding="utf-8")
    print(f"已生成 {len(directories)} 个目录页面：{root}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", nargs="?", default="files", help="文件根目录（默认: files）")
    args = parser.parse_args()
    generate(Path(args.directory))
