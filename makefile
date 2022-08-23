all : web_client DEBUG

restart : clean web_client DEBUG run_debug

web_client:
	gcc web_client.c -o web_client -lssl -lcrypto

DEBUG:
	gcc web_client.c -o web_client_debug -lssl -lcrypto -D _DEBUG

run_debug:
	./web_client_debug https://www.openfind.com.tw:443/ webpage/ 2

run_debug1:
	./web_client_debug https://www.openfind.com.tw:443/taiwan/markettrend_detail.php?news_id=24799 webpage/ 1

clean:
	rm -rf web_client web_client_debug data webpage temp