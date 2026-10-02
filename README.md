# Netease Mod Injector 2

#### 一个功能强大的网易Minecraft组件注入器

<div align="center">
    <img src="https://img.shields.io/github/last-commit/slfftz2011/NeteaseModInjector2" alt="GitHub last commit"/>
    <img src="https://img.shields.io/github/commit-activity/w/slfftz2011/NeteaseModInjector2" alt="GitHub commit activity"/>
    <img src="https://img.shields.io/github/contributors/slfftz2011/NeteaseModInjector2" alt="GitHub contributors"/>
    <br>
    <img src="https://img.shields.io/github/languages/code-size/slfftz2011/NeteaseModInjector2" alt="GitHub code size in bytes"/>
    <img src="https://img.shields.io/endpoint?url=https://ghloc.vercel.app/api/slfftz2011/NeteaseModInjector2/badge?filter=.cpp$&label=lines%20of%20code&color=blue" alt="GitHub lines of code"/>
    <br>
    <img alt="Total Downloads" src="https://img.shields.io/packagist/dt/slfftz2011/NeteaseModInjector2">
    <img alt="Latest Version" src="https://img.shields.io/packagist/v/slfftz2011/NeteaseModInjector2">
    <img alt="License" src="https://img.shields.io/github/license/slfftz2011/NeteaseModInjector2">
</div>

---

### 简介

ModInjector2 是网易Minecraft组件注入器的第二代版本，相比第一代，它支持组件包(.COP)文件，提供更安全和完整的mod注入体验。

### 新特性 (v2.0.0)

- **组件包支持**: 使用.COP格式的组件包，包含mods、config和resourcepacks
- **完整性验证**: 自动验证组件文件的完整性和正确性
- **备份功能**: 注入前自动备份现有文件
- **改进的错误处理**: 更详细的错误信息和用户反馈
- **网络检查**: 自动检测网络连接状态
- **MCI 联机下载**: 搜索 Modrinth 项目、按版本和加载器筛选并通过 MCI CDN 下载文件

### 下载

  [蓝奏云](https://pc.woozooo.com) 不限速下载支持

**密码: `1145`**

- [v2.0.0-rc1](https://wwxd.lanzouw.com/iBV0X37haaxg)
- [v2.0.0-rc2]() <- *待发布*
- 埋头苦干ing...

### 构建

需要 CMake、Ninja 和 MinGW-w64，构建命令如下：

```bash
cmake -S . -B cmake-build -G Ninja -DCMAKE_CXX_COMPILER=g++ -DCMAKE_RC_COMPILER=windres
cmake --build cmake-build --config Release
```

也可以使用 Visual Studio 的 CMake 生成器。程序菜单中的 MCI 下载会将文件保存到 `downloads/MCI/`。

### 使用方法

1. **准备组件**: 将.COP格式的组件文件放入 `components/` 目录
2. **运行程序**: 启动 ModInjector2.exe
3. **选择组件**: 从列表中选择要注入的组件
4. **验证**: 程序会自动验证组件完整性
5. **备份**: 自动备份现有mods、config和resourcepacks
6. **注入**: 启动游戏，程序会自动注入组件


### 作者的话

1. 首先灰常感谢 **闪烁的红石君** 提供技术支持 （->[点此支持原作者](https://mc.netease.com/forum.php?mod=viewthread&tid=990081&page=1&ordertype=1#pid5040389)<-）

2. 第二代终于来了！相比第一代，第二代支持组件包，验证完整性，还有备份功能，更安全可靠。

3. 如果发现bug或想为这个项目添砖加瓦，可以[提交问题](https://github.com/slfftz2011/ModInjector2/issues/new)和[创建拉取请求](https://github.com/slfftz2011/ModInjector2/compare)，在此万分感谢


#### 关于组件包

未来一段时间我会把开发者客户端做出来，届时你们就可以快速制作分享了

当然，如果你非常想要的话，联系我，给你定制一个（邮箱：`slfftz520@163.com`)

---

- 持续更新中... ——25/10/2 11:45
