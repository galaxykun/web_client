all : web_client DEBUG

web_client:
	gcc web_client.c -o web_client -lssl -lcrypto

DEBUG:
	gcc web_client.c -o web_client_debug -lssl -lcrypto -D _DEBUG

run_debug:
	./web_client_debug https://www.openfind.com.tw:443/taiwan/mailbase.html webpage/ 2


clean:
	rm -r web_client web_client_debug