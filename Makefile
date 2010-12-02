PREFIX=/usr/local/
CXXFLAGS=-I$(PREFIX)/include/opal -I$(PREFIX)/include/ptclib -I$(PREFIX)/include -I$(PREFIX)/ptlib -g
CPPFLAGS=-D__LINUX__ -DDISABLE_VIDEO

CORE_OBJECTS =	\
pcss.o		\
sip.o		\
state.o		\
telekarma.o	\
telephony.o	\
controller.o	\
view.o		\
model.o		\
action.o	\
account.o	\
sms.o

CORE_LIBS =	\
-lopal		\
-lpt

WX_GTK_LIBS=		\
-lwx_gtk2u_richtext-2.8 \
-lwx_gtk2u_aui-2.8	\
-lwx_gtk2u_xrc-2.8	\
-lwx_gtk2u_qa-2.8	\
-lwx_gtk2u_html-2.8	\
-lwx_gtk2u_adv-2.8	\
-lwx_gtk2u_core-2.8	\
-lwx_baseu_xml-2.8	\
-lwx_baseu_net-2.8	\
-lwx_baseu-2.8

tk: main.o cliview.o clicontext.o $(CORE_OBJECTS)
	g++ -o $@ $^ $(CORE_LIBS)  -L$(PREFIX)/lib

tk-gui: gui.o $(CORE_OBJECTS)
	g++ -o  $@ $^  $(WX_GTK_LIBS) $(CORE_LIBS) -L$(PREFIX)/lib

gui.o: gui.cpp
	g++ `wx-config --cppflags` $(CXXFLAGS) -c -o $@ $^
