# TODO

- 添加mica模式
- 程序退出时释放所有已加载的dll
- 在win10 19h1之前的系统上用d2d模糊壁纸
- 迁移到WinUI2，并在此仓库提供最新版cppwinrt生成的头文件，做到彻底不依赖外部环境
- 添加自动编译脚本和cmake工具链
- 处kernel32里的函数以外，所有函数都动态加载，最后只依赖kernel32.dll
