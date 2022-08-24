all : web_client DEBUG

restart : clean web_client DEBUG run_debug

web_client:
	gcc web_client.c -o web_client -lssl -lcrypto

DEBUG:
	gcc web_client.c -o web_client_debug -lssl -lcrypto -D _DEBUG

run_debug:
	./web_client_debug "https://www.openfind.com.tw/" webpage/ 5

run_debug1:
	./web_client_debug "https://www.openfind.com.tw:443/taiwan/markettrend_detail.php?news_id=24799" webpage/ 2

run_debug2:
	./web_client_debug "http://www.kingbus.com.tw/" webpage/ 2

run_debug3:
	./web_client_debug "https://www.storm.mg/lifestyle/4472684"  webpage/ 1

run_debug4:
	./web_client_debug "https://hackmd.io/@HsuChiChen/gcc-command"  webpage/ 1

run_debug5:
	./web_client_debug "https://www.google.com.tw/imghp?hl=zh-TW&tab=ri&ogbl"  webpage/  3

run_debug6:
	./web_client_debug "https://www.openfind.com.tw/taiwan/markettrend.php"  webpage/  1


clean:
	rm -rf web_client web_client_debug data webpage temp