#include <cstdio>
#include <cerrno>
#include <thread>
#include <zookeeper.h>

using namespace std;

// 连接状态跟踪
static int connected = 0;
static int expired = 0;

// zkHandler
static zhandle_t *zkHandler;

namespace fn {

    /**
     * 连接事件处理
     */
    void zookeeper_init_watcher(zhandle_t *zkH, int type, int state, const char *path, void *watcherCtx) {
        if (type == ZOO_SESSION_EVENT) {
            // 连接状态处理
            if (state == ZOO_CONNECTED_STATE) {
                // 连接成功；
                connected = 1;
            } else if (state == ZOO_NOTCONNECTED_STATE) {
                // 连接失败；
                connected = 0;
            } else if (state == ZOO_EXPIRED_SESSION_STATE) {
                // 会话过期；
                expired = 1;
                connected = 0;
                zookeeper_close(zkH);
            } else if (state == ZOO_AUTH_FAILED_STATE) {
                // 认证失败；
            }
        }
    }

    /**
     * ASYNC 回调函数
     * @param return_code 错误码
     * @param return_value 返回值
     * @param data 数据
     */
    void create_completion(int return_code, const char *return_value, const void *data) {
        if (return_code == ZOK) {
            // 成功
            fprintf(stdout, "Return Code: %d, Return Value: %s, Data: %s.\n", return_code, return_value, data);
        } else if (return_code == ZNODEEXISTS) {
            // 节点存在
            fprintf(stdout, "The node already exists");
        }
    }
}

int main() {
    // 设置 zookeeper 日志级别；
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    // zookeeper 建立连接
    zkHandler = zookeeper_init("127.0.0.1:2181", fn::zookeeper_init_watcher, 10000, 0, 0, 0);

    if (!zkHandler) {
        return errno;
    } else {
        fprintf(stderr, "Connection established with Zookeeper. \n");
    }

    // 节点
    const char node[] = "/node";
    // 节点值
    const char node_value[] = "node_value";
    // 节点值长度
    int node_value_len = sizeof(node_value);

    //
    const char data[] = "Passthrough";
    // ASYNC 创建节点
    zoo_acreate(zkHandler,                          // ZK 句柄
                node,                               // 节点
                node_value,                         // 节点值
                node_value_len,                     // 节点值长度
                &ZOO_OPEN_ACL_UNSAFE,               // OPEN ACL
                ZOO_EPHEMERAL,                      // 临时节点模式
                fn::create_completion,              // 回调
                data                                // 需要传递给回调函数的数据
    );

    std::this_thread::sleep_for(2s);

    // zookeeper 释放连接
    zookeeper_close(zkHandler);

    return 0;
}
