#include <stdio.h>
#include <unistd.h>
#include <zookeeper/zookeeper.h>

void test_watcher(zhandle_t *zh, int type,
        int state, const char *path,void *watcherCtx) {
	char buf[512] = {0};
	int buflen = 512;
	zhandle_t *h = (zhandle_t *)watcherCtx;
	printf("triggered watcher type[%d] stat[%d] path[%s]\n",
			type, state, path);
	if (ZOO_CONNECTED_STATE == state) {
		zoo_get(zh, "/root", 1, buf, &buflen, NULL);
		printf("/root: %s\n", buf);
	}
}

int main(int argc, char *argv[]) {
	int res = -1;
	zhandle_t *h = NULL;
	h = zookeeper_init("127.0.0.1:2181", test_watcher, 50000, NULL, (void *)12345, 0);

	sleep(100000);
}
