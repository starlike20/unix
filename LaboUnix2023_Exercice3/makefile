.SILENT:

UNIX_Exercice3:	main.o mainwindowex3.o moc_mainwindowex3.o CreationBD Lecture
	echo creation de UNIX_Exercice3
	g++ -o UNIX_Exercice3 main.o mainwindowex3.o moc_mainwindowex3.o   /usr/lib64/libQt5Widgets.so /usr/lib64/libQt5Gui.so /usr/lib64/libQt5Core.so /usr/lib64/libGL.so -lpthread
main.o:	main.cpp
	echo creation de main.o
	g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_Exercice1 -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o main.o main.cpp
mainwindowex3.o:	mainwindowex3.cpp
	echo cretion de mainwindowex3.o 
	g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_Exercice1 -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o mainwindowex3.o mainwindowex3.cpp
moc_mainwindowex3.o:	moc_mainwindowex3.cpp
	echo creation de moc_mainwindowex3.o 
	g++ -c -pipe -g -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -DQT_DEPRECATED_WARNINGS -DQT_QML_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB -I../UNIX_Exercice1 -I. -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtWidgets -isystem /usr/include/qt5/QtGui -isystem /usr/include/qt5/QtCore -I. -I. -I/usr/lib64/qt5/mkspecs/linux-g++ -o moc_mainwindowex3.o moc_mainwindowex3.cpp
CreationBD:
	echo creation de CreationBD
	g++ -o CreationBD CreationBD.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
Lecture:
	echo creation de Lecture
	g++ -o Lecture Lecture.cpp -I/usr/include/mysql -m64 -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
