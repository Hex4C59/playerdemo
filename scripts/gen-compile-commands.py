#!/usr/bin/env python3
"""为 clangd 生成 compile_commands.json。

项目的真实构建走 qmake + nmake（MSVC），qmake 不产出 compile_commands.json，
所以这里按 Makefile.Release 的 INCPATH / DEFINES 复刻一份等价的编译命令。

用法：
    python scripts/gen-compile-commands.py

改动 Qt 路径或新增源文件后重新运行即可。生成的 compile_commands.json
已加入 .gitignore，不进版本库。
"""
import json
import os
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
QT_ROOT = Path(r"D:\Dev\SDKs\Qt\6.8.3\msvc2022_64")

# 与 Makefile.Release 的 DEFINES 保持一致
DEFINES = [
    "UNICODE",
    "_UNICODE",
    "WIN32",
    "_ENABLE_EXTENDED_ALIGNED_STORAGE",
    "WIN64",
    "QT_WIDGETS_LIB",
    "QT_GUI_LIB",
    "QT_CORE_LIB",
]

# 与 Makefile.Release 的 CXXFLAGS 保持一致（去掉 MSVC 专有、clang 不认的项）
COMMON_FLAGS = [
    "-std=c++17",
    "-fms-extensions",       # 支持 #pragma execution_character_set 等 MS 扩展
    "-fms-compatibility",
    "-fdelayed-template-parsing",
    "-Wno-unknown-pragmas",
    "-Wno-microsoft-enum-forward-reference",
]

INCLUDE_DIRS = [
    PROJECT_ROOT,
    PROJECT_ROOT / "src",
    PROJECT_ROOT / "lib",
    PROJECT_ROOT / "lib" / "ffmpeg" / "include",
    PROJECT_ROOT / "lib" / "SDL2",
    QT_ROOT / "include",
    QT_ROOT / "include" / "QtWidgets",
    QT_ROOT / "include" / "QtGui",
    QT_ROOT / "include" / "QtCore",
    QT_ROOT / "mkspecs" / "win32-msvc",
]

# moc/uic 生成的头文件在 release/ 下（构建后才有，缺失不影响 clangd）
EXTRA_INCLUDE_DIRS = [PROJECT_ROOT / "release"]


def find_clang() -> str:
    """返回 clang++ 路径，用于 compile_commands 的 compiler 字段。

    clangd 只从 --driver-mode / compiler 推断目标平台和系统头文件位置，
    它不会真的调用这个编译器。
    """
    for name in ("clang++.exe", "clang++"):
        found = None
        try:
            out = subprocess.run(["where", name], capture_output=True, text=True)
            if out.returncode == 0:
                found = out.stdout.strip().splitlines()[0]
        except FileNotFoundError:
            pass
        if found:
            return found
    return "clang++.exe"


def main() -> int:
    if not QT_ROOT.exists():
        print(f"错误：找不到 Qt 目录 {QT_ROOT}", file=sys.stderr)
        print("请修改脚本顶部的 QT_ROOT 后重试。", file=sys.stderr)
        return 1

    sources = sorted(PROJECT_ROOT.glob("src/*.cpp"))
    if not sources:
        print("错误：src/ 下没有找到 .cpp 文件", file=sys.stderr)
        return 1

    compiler = find_clang()

    args = [compiler]
    args += COMMON_FLAGS
    args += [f"-D{d}" for d in DEFINES]
    for d in INCLUDE_DIRS:
        args.append(f"-I{d}")
    for d in EXTRA_INCLUDE_DIRS:
        args.append(f"-I{d}")

    entries = []
    for src in sources:
        entries.append(
            {
                "directory": str(PROJECT_ROOT),
                "file": str(src),
                "output": str(PROJECT_ROOT / "release" / (src.stem + ".obj")),
                "arguments": args + ["-c", str(src)],
            }
        )

    out_file = PROJECT_ROOT / "compile_commands.json"
    out_file.write_text(json.dumps(entries, indent=2, ensure_ascii=False), encoding="utf-8")

    print(f"已生成 {out_file}")
    print(f"  源文件 {len(entries)} 个")
    print(f"  头文件目录 {len(INCLUDE_DIRS) + len(EXTRA_INCLUDE_DIRS)} 个")
    print(f"  宏定义 {len(DEFINES)} 个")
    print()
    print("下一步：在 Zed 里执行 \"editor: restart language server\"，")
    print("或直接重启 Zed 让 clangd 重新读取配置。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())