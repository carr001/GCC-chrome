CUR_DIR=$(shell pwd)
OUTDIR:=$(CUR_DIR)/out/
BIN_DIR:=$(CUR_DIR)/bin/
INCLUDEPATH:= $(CUR_DIR)/include/

CXX=g++
HEADPATH:=$(CUR_DIR) $(INCLUDEPATH)
CFLAGS:=-std=c++17 -g -c
VPATH=$(shell find src -type d)# recurive path search need help from shell
LDLIBS := -lglog
GLOG_INSTALL_DIR = /usr/local/lib
LDFLAGS := -Wl,-rpath -Wl,$(GLOG_INSTALL_DIR)

TARGET=$(BIN_DIR)sender_test $(BIN_DIR)receiver_test $(BIN_DIR)packet_parser_test $(BIN_DIR)packet_maker_test

RED = \\e[1m\\e[31m
RESET = \\e[m

DEPS:=bitrate_estimator.o\
goog_cc_network_control.o\
frequency.o\
time.o\
bwe_defines.o\
utils.o\
quic_time.o\
trendline_estimator.o\
timestamp.o\
data_size.o\
quic_chromium_clock.o\
quic_clock.o\
acknowledged_bitrate_estimator_interface.o\
acknowledged_bitrate_estimator.o\
delay_based_bwe.o\
network_types.o\
link_capacity_estimator.o\
aimd_rate_control.o\
time_delta.o\
send_side_bandwidth_estimation.o\
inter_arrival_delta.o\
data_rate.o\
packet.o\
packet_maker.o\
packet_parser.o\
sender.o\
receiver.o\
receiver_loss_collection.o\


DEPS2=$(patsubst %.o,$(OUTDIR)%.o,$(DEPS) )

sender_test:$(BIN_DIR)sender_test ; #default is sender_test

receiver_test:$(BIN_DIR)receiver_test ;

packet_parser_test:$(BIN_DIR)packet_parser_test ;

packet_maker_test:$(BIN_DIR)packet_maker_test ;

$(BIN_DIR)sender_test:$(DEPS2)  $(OUTDIR)sender_test.o
	$(CXX) $(LDFLAGS) -o $@  $^ $(LDLIBS)
	@echo $(RED) $@ build Done! $(RESET)

$(BIN_DIR)receiver_test:$(DEPS2) $(OUTDIR)receiver_test.o
	$(CXX) $(LDFLAGS) -o $@  $^ $(LDLIBS)
	@echo $(RED) $@ build Done! $(RESET)

$(BIN_DIR)packet_parser_test:$(DEPS2) $(OUTDIR)packet_parser_test.o
	$(CXX) $(LDFLAGS) -o $@  $^ $(LDLIBS)
	@echo $(RED) $@ build Done! $(RESET)

$(BIN_DIR)packet_maker_test:$(DEPS2) $(OUTDIR)packet_maker_test.o
	$(CXX) $(LDFLAGS) -o $@  $^ $(LDLIBS)
	@echo $(RED) $@ build Done! $(RESET)

$(OUTDIR)%.o:%.cc
	$(CXX) $(CFLAGS) $^ -o $@ -I $(HEADPATH)

.PHONY:clean
clean:
	rm $(DEPS2) $(TARGET) $(OUTDIR)sender_test.o $(OUTDIR)receiver_test.o
