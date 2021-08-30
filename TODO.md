# TODO

- 添加mica模式
- 程序退出时释放所有已加载的dll
- 迁移到WinUI2，并在此仓库提供最新版cppwinrt生成的头文件，做到彻底不依赖外部环境
- 迁移到D3D11on12和D3D12on7
- 所有能使用constexpr的地方都使用
- winui3发布后添加对应后端
- 添加Windows.UI.Composition后端
- 调查Microsoft Edge Update Installer (Google Chrome Update Installer) 是如何在win7系统上也保留系统边框的
- 后端所有的函数执行前都先判断下是否应直接返回
- DWM 函数失败后直接读取注册表，尽量返回一个有效值而不是直接报错
- dynamicapis只包含基本的win32函数（user32、shell32、gdi32等），图形函数移动到对应的后端去加载
