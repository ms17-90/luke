# luke
自己开发的压力测试程序，不过对网站的威胁很大，希望大家不要乱用，仅供学习参考
# 如何安装
git clone https://github.com/ms17-90/luke.git
cd luke
# 编译源码
gcc -o luck.c luck -lcurl -lpthread
# 编译完成的可执行二进制程序到系统级目录
sudo mv /home/android/luke/src/luke /usr/local/bin #补充一点 把android替换成你们的用户名
bash ：
luck 目标网站 用户数量 请求数量 # 回车一下就可以执行了
# 开发者说的话
debin&Ubunto
Arch&arco
Gentoo&Pentoo
这些发行版都可以！
