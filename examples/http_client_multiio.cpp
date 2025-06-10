/*
 * @build   make examples
 *
 * @server  bin/http_server_test 8080
 * @client  bin/http_client_multiio [requests] [threads]
 */

#include "hv.h"
#include "HttpClient.h"
#include "EventLoopThreadPool.h"

#include <vector>
#include <atomic>

using namespace hv;

int main(int argc, char** argv) {
    int req_cnt = 10;
    int threads = 4;
    if (argc > 1) req_cnt = atoi(argv[1]);
    if (argc > 2) threads = atoi(argv[2]);

    EventLoopThreadPool loop_threads(threads);
    loop_threads.start(true);

    std::vector<std::shared_ptr<AsyncHttpClient>> clients;
    for (int i = 0; i < threads; ++i) {
        EventLoopPtr loop = loop_threads.loop(i);
        clients.push_back(std::make_shared<AsyncHttpClient>(loop));
    }

    std::atomic<int> resp_cnt(0);

    for (int i = 0; i < req_cnt; ++i) {
        auto req = std::make_shared<HttpRequest>();
        req->method = HTTP_GET;
        req->url = "http://127.0.0.1:8080/echo";
        req->body = std::string("req ") + std::to_string(i);
        clients[i % threads]->send(req, [&resp_cnt](const HttpResponsePtr& resp) {
            if (resp) {
                printf("%d %s\n", resp->status_code, resp->body.c_str());
            } else {
                printf("request failed\n");
            }
            ++resp_cnt;
        });
    }

    while (resp_cnt < req_cnt) {
        hv_delay(100);
    }

    loop_threads.stop(true);
    return 0;
}
