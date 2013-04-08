#f = open('/media/sda11/Python/Python Projects/texteditor/compiler.py','r')
#f = open('/media/sda11/C/GTK+/gIDLE/main.c','r')
#f = open('/media/sda11/C/GTK+/gIDLE/codewidget.c')
f = open('/media/sda11/GSoC/musicbrainz_test.c')
#f = open('/media/sda11/C++/csharp/proj.cpp','r')
#f = open('/media/sda6/C/argc.c','r')
#f = open('/media/sda11/C++/gtkmm/csharp/proj.h','r')
#f = open('/media/sda11/C++/csharp/proj.h','r')
#f=open('/media/sda11/C++/gtkmm/gtk_creator/gtk_vbox.cpp','r')
#f=open('/home/abhi/Desktop/nn/nn.glade','r')
s = ''
for d in f:
    s = s +d
 
import re

#print re.findall(r'\w+\s+main_window::+.+\(.*\)\s*{',s)
print re.findall(r'[\w*]+[\s*]+[\w*]+[\s*]*\(+.*\)\s*{',s)
#print re.findall(r'[\btemplate\s*<class\s+\w+\s*>]*\bclass\b\s*\w+.+?\}\s*\;',s,re.DOTALL)

#print re.findall(r'.+\{',s,re.DOTALL)
#print re.findall(r'[\w*]+[\s*]+[\w*]+\(+.+\)\s*;',s)
#print re.findall(r'class=+.+id=+.+',s)#,re.DOTALL)
##s="""Total Size to download 71346
##downloaded by part0 0 start_size 0 end_size 14269
##downloaded by part1 0 start_size 14269 end_size 28538
##downloaded by part2 0 start_size 28538 end_size 42807
##downloaded by part3 0 start_sizen 42807 end_size 57076
##downloaded by part4 0 start_size 57076 end_size 71346
##total download 0
##speed 0"""
##print re.findall(r'Total Size to download\s*([0-9]*)',s)
##print re.findall(r'downloaded by part[0-9]*\s*([0-9]*)',s)
##print re.findall(r'start_size\s*([0-9]*)\s*end_size\s*([0-9]*)',s)
##print re.findall(r'speed\s*([0-9]*)',s)
#print re.findall(r'gtk_vbox::,\s*(\w\(*.*\))\s*{',s)
#print re.findall(r'gtk_container::\s*(\w\(*.*\))\s*{',s)
#print re.findall(r'\}\s*;',s)
#print re.findall(r'main_window::~main_window\(*.*\)',s)
#print re.findall(r'(?<=\}\;)\s*\w+.+?(?=\}\;)',s,re.DOTALL)
#print re.findall(r'(\w+.+)\s*\{',s)
#print re.findall(r'[\w*]+[\s*]+[\w*]+\(+.+\)\s*\{',s)
#print re.findall(r'[\w*]+[\s*]+[\w*]+\(+.+\)\s*\;',s)
#print re.findall(r'(?<=\{)\s*\w+.+?(?=\})',s,re.DOTALL)
#print re.findall(r'\benum\b(.+?\}\s*\;)',s,re.DOTALL)
#print re.sub(r'\benum\b(.+?\}\s*\;)'," ",s,re.DOTALL)
#print re.findall(r'[\ \t]*\bclass\b\s*\w+',s)
#print re.findall(r'>\s+\bclass\b\s*\w+',s)
#print re.findall(r'([^\n][\ \t]*\bdef\b\s*.+)\:',s)
#print re.findall(r'(\bself\.+.+)\=',s)
#print re.findall(r'\bvoid\b\s*(\w+.+)\s*{',s)
#print re.findall(r'\bmain_window::main_window*.*',s)
#print re.findall(r'\b%s\b\s*(\w+.+)\s*?'%('void'),s)
#print re.findall(r'\b%s\s*%s'%('void','destroy\(GtkWidget \*\)'),s)
#print re.findall(r'\b%s::%s\s*\{'%('main_window','filesaveall_activated()'),s,re.DOTALL)
#print re.findall(r'/\*+.+\*/',s,re.DOTALL)
#print re.findall(r'\b%s\s*%s::%s'%('void','main_window','formatindentregion_activated\(\)'),s)
#print re.findall(r'\b%s\s*%s\s*\;'%('virtual void','formatindentregion_activated\(\)'),s)
#print re.findall(r'\s+(.+\w+.+)[^\n]',s)
#print re.findall(r'\bdef\b\s*getcommand\(self\)',s)
