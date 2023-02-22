进入build目录 执行 cmake ..  就是找最外层的makelists文件去编译一层一层往下找，把生成的文件都放在build里面
执行完毕，执行make，可执行文件生成到bin目录下
重新编译，就是进入build目录 rm -rf * 删除所有文件，重新cmake ..编译  make
