all : web_client DEBUG

web_client:
	gcc web_client.c -o web_client -lssl -lcrypto

DEBUG:
	gcc web_client.c -o web_client_debug -lssl -lcrypto -D _DEBUG

run_debug:
	./web_client_debug https://www.openfind.com.tw/taiwan/mailbase.html webpage/


clean:
	rm -r web_client web_client_debug