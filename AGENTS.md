# AGENTS.md

本文件记录本仓库的协作约定，供人和 AI 助手共同遵守。

## 仓库性质

`playerdemo` 的上游是 [itisyang/playerdemo](https://github.com/itisyang/playerdemo)。
当前仓库是个人 fork（`Hex4C59/playerdemo`）。

## Git 提交规则

### 只推送到 fork，不向上游提 PR

**所有改动只提交并推送到 fork 仓库 `origin`（`Hex4C59/playerdemo`）。**

- 不向上游 `itisyang/playerdemo` 提交 Pull Request
- 不直接向上游仓库推送
- 不请求上游维护者 review

### 远程仓库布局

| remote | 地址 | 用途 |
| --- | --- | --- |
| `origin` | `https://github.com/Hex4C59/playerdemo.git` | 个人 fork，**所有改动推送到此** |
| `upstream` | `https://github.com/itisyang/playerdemo.git` | 上游原始仓库，**只读**，用于同步 |

`upstream` 仅用于获取上游更新（`git fetch upstream`），不向其推送。

### 提交信息格式

沿用仓库既有风格：英文 type 前缀 + 中文正文。

```
fix(ui): 接通主菜单动作、修复标题栏菜单与主窗口拖放
fix: 修正 Decoder 的 memset 未定义行为与 Qt 6.8 弃用 API
chore: 为 clangd 生成编译数据库，忽略构建产物
```

- type 常用：`feat`、`fix`、`chore`、`docs`、`refactor`
- scope 可选，标注影响范围，如 `fix(ui)`、`fix(audio)`
- 正文说明**为什么改**，而非重复代码做了什么
- 一个提交只做一件事；不相关改动拆成多个提交

### 网络代理

本机通过 `127.0.0.1:7897` 访问 GitHub。git 不会自动读取系统代理，
若 `git fetch/push` 报 443 超时，确认本仓库已配置：

```bash
git config http.proxy  http://127.0.0.1:7897
git config https.proxy http://127.0.0.1:7897
```

## 构建环境

| 项目 | 版本 / 路径 |
| --- | --- |
| Qt | 6.8.3 msvc2022_64，位于 `D:\Dev\SDKs\Qt\6.8.3\msvc2022_64` |
| 编译器 | MSVC 19.51（VS 2026），x64 |
| FFmpeg | 5.1 shared x64，DLL 放在 `bin/` |
| SDL2 | 2.26.2 x64，DLL 放在 `bin/` |
| 构建方式 | qmake + nmake（`playerdemo.pro`） |

构建步骤：

```bash
call "D:\APP\code\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
set PATH=D:\Dev\SDKs\Qt\6.8.3\msvc2022_64\bin;%PATH%
qmake
nmake release
windeployqt --no-translations --no-compiler-runtime bin\playerdemo.exe
```

`bin/`、`dist/`、qmake 构建产物均已在 `.gitignore` 中排除，不入库。

## 编辑器（Zed / clangd）

项目用 qmake，不产出 `compile_commands.json`，clangd 因此找不到
Qt / SDL2 / FFmpeg 头文件，编辑器会满屏报 `file not found`。

生成方式：

```bash
python scripts/gen-compile-commands.py
```

升级 Qt 后修改脚本顶部的 `QT_ROOT` 再重新运行。

## 代码约定

- 对外可见的注释用中文，与既有源码一致
- 不修改 `lib/ffmpeg`、`lib/SDL2` 中的第三方头文件与导入库
- `src/datactl.h` 中的结构体若含 `std::thread` 等非平凡类型，
  不要用 `memset` 整体清零（未定义行为），逐字段初始化