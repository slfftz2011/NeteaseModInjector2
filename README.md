
<h1 align="center">Netease Mod Injector 2</h1>
<p align="center">一个功能强大的网易Minecraft组件注入器.</p>

<div align="center">

[![Language](https://img.shields.io/badge/c++-blue)]()

[![Github last commit](https://img.shields.io/github/last-commit/slfftz2011/NeteaseModInjector2)](https://github.com/slfftz2011/NeteaseModInjector2/commits/)
[![Github commit activity](https://img.shields.io/github/commit-activity/w/slfftz2011/NeteaseModInjector2)](https://github.com/slfftz2011/NeteaseModInjector2/activity/)
[![Github contributors](https://img.shields.io/github/contributors/slfftz2011/NeteaseModInjector2)](https://github.com/slfftz2011/NeteaseModInjector2/contributors/)

![Github code size](https://img.shields.io/github/languages/code-size/slfftz2011/NeteaseModInjector2)
![GitHub repo size](https://img.shields.io/github/repo-size/slfftz2011/NeteaseModInjector2)
![Github lines of code](https://5ezz6jithh.execute-api.us-east-1.amazonaws.com/prod/lambda-shield-redirect?user=slfftz2011&repo=NeteaseModInjector2)

[![GitHub Downloads](https://img.shields.io/github/downloads/slfftz2011/NeteaseModInjector2/total)](https://github.com/slfftz2011/NeteaseModInjector2/releases/)
[![GitHub Tag](https://img.shields.io/github/v/tag/slfftz2011/NeteaseModInjector2)](https://github.com/slfftz2011/NeteaseModInjector2/releases/)
![GitHub Repo stars](https://img.shields.io/github/stars/slfftz2011/NeteaseModInjector2)
</div>

---

### 简介

ModInjector2 是网易Minecraft组件注入器的第二代版本，相比[第一代](https://github.com/slfftz2011/NeteaseModInjector2/tree/v1)，它支持组件包(.COP)文件，提供更安全和完整的mod注入体验。

### 新特性 (v2.0.0)

- **组件包支持**: 使用.COP格式的组件包，包含mods、config和resourcepacks
- **完整性验证**: 自动验证组件文件的完整性和正确性
- **备份功能**: 注入前自动备份现有文件
- **改进的错误处理**: 更详细的错误信息和用户反馈
- **网络检查**: 自动检测网络连接状态
- **MCI 联机下载**: 搜索 Modrinth/CurseForge 项目、按游戏版本和加载器筛选并通过 MCI CDN 下载文件

### 下载

  [蓝奏云](https://pc.woozooo.com) 不限速下载支持

**密码: `1145`**

- [全部版本](https://github.com/slfftz2011/NeteaseModInjector2/releases)
- [v2.0.0-rc1](https://wwxd.lanzouw.com/iBV0X37haaxg)
- [v2.0.0-rc2](https://slfftz2011.lanzouw.com/izLWH4an452j)
- [v2.0.0-rc3](https://slfftz2011.lanzouw.com/ic4Co4an4dch)
- 埋头苦干ing...

### 构建

需要 CMake 3.20+、Ninja 和 MinGW-w64，并确保 `g++`、`windres`、`cmake`、`ninja` 在 `PATH` 中。在项目根目录运行：

```bash
cmake -S . -B cmake-build -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_RC_COMPILER=windres
cmake --build cmake-build --config Release
```

生成的程序位于 `cmake-build/bin/ModInjector2.exe`。也可以使用 Visual Studio 的 CMake 生成器。菜单中的 MCI 下载支持 Modrinth 和 CurseForge，文件保存到 `downloads/MCI/`。

### 使用方法

1. **准备组件**: 将.COP格式的组件文件放入 `components/` 目录
2. **运行程序**: 启动 ModInjector2.exe
3. **选择组件**: 从列表中选择要注入的组件
4. **验证**: 程序会自动验证组件完整性
5. **备份**: 自动备份现有mods、config和resourcepacks
6. **注入**: 启动游戏，程序会自动注入组件


### 作者的话

1. 首先灰常感谢 **闪烁的红石君** 提供技术支持 （->[点此支持原作者](https://mc.netease.com/forum.php?mod=viewthread&tid=990081&page=1&ordertype=1#pid5040389)<-）

  > 注：链接已失效

感谢 [MciMirror](https://www.mcimirror.top/) 提供国内Modrinth/CurseForge信息镜像服务

2. 第二代终于来了！相比第一代，第二代支持组件包，验证完整性，还有备份功能，更安全可靠。

3. 如果发现bug或想为这个项目添砖加瓦，可以[提交问题](https://github.com/slfftz2011/ModInjector2/issues/new)和[创建拉取请求](https://github.com/slfftz2011/ModInjector2/compare)，在此万分感谢

4. 本项目现已作为 [Ibuprofen Loader（即Netease Mod Loader 3）](https://github.com/slfftz2011/Ibuprofen-Loader)的纯终端版本（c++）以及功能测试项目进行维护，适用不需要前端UI的用户及开发者使用，存在大量实验性功能，如有Bug提交 [Issues](https://github.com/slfftz2011/NeteaseModInjector2/issues)，在未来的某一天将 **不再维护并归档** !

---

- 持续更新中... ——25/10/2 11:45
- 突然诈尸! ——26/10/2 12:54
