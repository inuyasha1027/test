#include <sys/time.h> // Need before osip.h.
#include <osip2/osip.h>
#include <osip2/osip_mt.h>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static int g_socket_fd = -1;
static int g_send_fd = -1;
static osip_t* g_osip = NULL;
static const size_t BUFFSIZE = 1600;  // MTU usually doesn't exceed 1600
static const unsigned MESSAGE_ENTRY_MAX_LENGTH = 256;
static const unsigned MAX_ADDR_STR = 128;

void exit_if_failed(int ret, const std::string& info) {
    if (ret != 0) {
        std::cout << "exit_if_failed:" << ret << " ," << info << std::endl;
        exit(-1);
    }
}

void cleanup_socket() {
    std::cout << "cleanup socket" << std::endl;
    if (g_socket_fd > 0) {
        close(g_socket_fd);
        g_socket_fd = -1;
    }
    if (g_send_fd > 0) {
        close(g_send_fd);
        g_send_fd = -1;
    }
}

void process_new_request(osip_t* osip, osip_event_t* evt)
{
    osip_transaction_t *tran;
    osip_transaction_init(&tran, IST, osip, evt->sip);
    //osip_transaction_set_in_socket (tran, socket);
    osip_transaction_set_out_socket (tran, g_send_fd);
    osip_transaction_set_your_instance(tran, osip);// store osip in transaction for later usage.
    osip_transaction_add_event(tran, evt);
}

void* transport_fun(void* arg) {
    std::cout << "transport_fun running" << std::endl;
    assert(g_send_fd > 0);

    osip_t* osip = static_cast<osip_t*>(arg);
    char buf[BUFFSIZE];
    while(true)
    {
        int len = read(g_send_fd, buf, BUFFSIZE);
        if (len == -1) {
            std::cout << "read error" << std::endl;
            exit(-1);
        }
        buf[len] = 0;
        std::cout << "message received:" << buf << std::endl;
        
        osip_event_t *evt = osip_parse(buf, len);
        if (evt == NULL) {
            std::cout << "osip_parse failed:" << buf << std::endl;
            return NULL;
        }

        int rc = osip_find_transaction_and_add_event(osip, evt);
        if(0 != rc) {
            std::cout << "this event has no transaction, create a new one.";
            process_new_request(osip, evt);
        }
    }

    return NULL;
}

// All Callbacks
// This function, which is implementation of transportation ourself, is used to send message.
int cb_send_message(osip_transaction_t * tr, osip_message_t * sip, char *host, int port, int out_socket);

void cb_xixt_kill_transaction(int type, osip_transaction_t * tr);

void cb_transport_error(int type, osip_transaction_t * tr, int error);

void cb_rcv1xx(int type, osip_transaction_t * tr, osip_message_t * sip);
void cb_rcv2xx(int type, osip_transaction_t * tr, osip_message_t * sip);
void cb_rcv3xx(int type, osip_transaction_t * tr, osip_message_t * sip);
void cb_rcv4xx(int type, osip_transaction_t * tr, osip_message_t * sip);
void cb_rcv5xx(int type, osip_transaction_t * tr, osip_message_t * sip);
void cb_rcv6xx(int type, osip_transaction_t * tr, osip_message_t * sip);

void cb_rcvreq(int type, osip_transaction_t * tr, osip_message_t * sip);

// Usage
void usage(const char* process)
{
    std::cout << process << " true/false\n" 
            << "  true  socket server. listen on localhost:7890\n"
            << "  false socket client. try to connect localhost:7890\n" << std::endl;
}

int init_net(bool as_server)
{
    if (as_server) {
        std::cout << "This is server" << std::endl;
    } else {
        std::cout << "This is client" << std::endl;
    }

    unsigned short port = 7890;
    const char* ip = "127.0.0.1";

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == fd) {
        printf("socket fail ! \n");
        return -1;
    }

    if (as_server) {
        g_socket_fd = fd;

        struct sockaddr_in s_add, c_add;
        bzero(&s_add,sizeof(struct sockaddr_in));
        s_add.sin_family = AF_INET;
        s_add.sin_addr.s_addr = htonl(INADDR_ANY);
        s_add.sin_port = htons(port);

        if(-1 == bind(fd, (struct sockaddr *)(&s_add), sizeof(struct sockaddr))) {
            printf("bind fail !\n");
            cleanup_socket();
            return -1;
        }

        if(-1 == listen(fd, 5)) {
            printf("listen fail !\n");
            cleanup_socket();
            return -1;
        }
        printf("listen ok\n");

        socklen_t sin_size = sizeof(struct sockaddr);
        int l_fd = accept(fd, (struct sockaddr *)(&c_add), &sin_size);
        if(-1 == l_fd)
        {
            printf("accept fail !\n");
            cleanup_socket();
            return -1;
        }
        printf("accept ok!\nServer start get connect from %s : %d\n", inet_ntoa(c_add.sin_addr), ntohs(c_add.sin_port));

        return l_fd;
    }

    // as client
    struct sockaddr_in add;
    bzero(&add,sizeof(struct sockaddr_in));
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = inet_addr(ip);
    add.sin_port = htons(port);

    if(-1 == connect(fd,(struct sockaddr *)(&add), sizeof(struct sockaddr))) {
        printf("connect fail! error:%s\n", strerror(errno));
        cleanup_socket();
        return -1;
    }
    printf("connect ok !\n");

    return fd;
}

int send_invite()
{
    static int g_seq_num = 1;
    char* seq_num_str = NULL;
    char temp[MESSAGE_ENTRY_MAX_LENGTH] = { 0 };
    char sip_port[MESSAGE_ENTRY_MAX_LENGTH] = { 0 };
    char expires[10];

    osip_call_id_t* call_id_ptr;
    osip_cseq_t* cseq_ptr;
    osip_message_t* invite_msg;

    int status = osip_message_init(&invite_msg);
    if (status != 0) {
        std::cout << "osip message init failed" << std::endl;
        return -1;
    }
    
    osip_message_set_method(invite_msg, osip_strdup("INVITE"));
    osip_uri_init(&(invite_msg->req_uri));
    if ((status = osip_uri_parse(invite_msg->req_uri, "sip:10.10.10.1")) != 0) {
        std::cout << "osip uri parse failed" << std::endl;
        osip_message_free(invite_msg);
        return -1;
    }

    osip_message_set_version(invite_msg, osip_strdup("SIP/2.0"));
    osip_message_set_status_code(invite_msg, 0);
    osip_message_set_reason_phrase(invite_msg, NULL);

    osip_message_set_to(invite_msg, "sip:Hal@10.10.10.2");
    osip_message_set_from(invite_msg, "sip:dafei@10.10.10.3");

    if ((status = osip_call_id_init(&call_id_ptr)) != 0) {
        std::cout << "call id failed" << std::endl;
        osip_message_free(invite_msg);
        return -1;
    }

    char* call_id_number_str = (char*)osip_malloc(MAX_ADDR_STR);
    unsigned number = osip_build_random_number();
    sprintf(call_id_number_str, "%u", number);
    osip_call_id_set_number(call_id_ptr, call_id_number_str);

    osip_call_id_set_host(call_id_ptr, osip_strdup("10.1.1.63"));

    invite_msg->call_id = call_id_ptr;

    if ((status = osip_cseq_init(&cseq_ptr)) != 0) {
        osip_message_free(invite_msg);
        return -1;
    }

    g_seq_num++;
    seq_num_str = (char*)osip_malloc(MAX_ADDR_STR);
    sprintf(seq_num_str, "%i", g_seq_num);
    osip_cseq_set_number(cseq_ptr, seq_num_str);
    osip_cseq_set_method(cseq_ptr, osip_strdup("INVITE"));
    invite_msg->cseq = cseq_ptr;

    osip_message_set_max_forwards(invite_msg, "70");
    sprintf(temp, "SIP/2.0/%s %s;branch=z9hG4bK%u", "UDP", "10.10.10.3", osip_build_random_number());
    osip_message_set_via(invite_msg, temp);

    osip_message_set_contact(invite_msg, "dafei@10.10.10.3");
    sprintf(expires, "%i", 3600);
    osip_message_set_expires(invite_msg, expires);

    osip_message_set_content_length(invite_msg, "0");
    osip_message_set_user_agent(invite_msg, "TotalView 1.0");

    // osip_message_set_allow(msgPtr, "INVITE");
    // osip_message_set_allow(msgPtr, "INFO");
    // osip_message_set_allow(msgPtr, "ACK");
    // osip_message_set_allow(msgPtr, "CANCEL");
    // osip_message_set_allow(msgPtr, "BYE");

    osip_transaction_t* transcation;
    osip_event_t* event;
    if ((status = osip_transaction_init(&transcation, ICT, g_osip, invite_msg)) != 0) {
        std::cout << "Failed to init transportation :" << status << std::endl;
        return -1; 
    }

    if ((event = osip_new_outgoing_sipmessage(invite_msg)) == NULL) {
        std::cout << "Can't allocate message" << std::endl;
        osip_message_free(invite_msg);
        return -1;
    }

    event->transactionid = transcation->transactionid;

    if((status = osip_message_force_update(invite_msg)) != 0){
        std::cout << "Failed force update:" << status << std::endl;
        osip_message_free(invite_msg);
        return -1;
    }

    if((status = osip_transaction_add_event(transcation, event)) != 0){
        std::cout << "Can't add event" << std::endl;
        osip_message_free(invite_msg);
        return -1;
    }

    return 0;
}

void* command_fun(void* arg)
{
    std::cout << "command thread is running" << std::endl;
    std::string command;
    std::cout << ">";
    while (std::getline(std::cin, command)) {
        if (command == "invite") {
            assert(g_osip);
            send_invite();
        } else {
            std::cout << "unknown command:" << command << std::endl;
        }

        std::cout << ">" << std::flush;
    }
    return NULL;
}

int main(int argc, char** argv){
    if (argc != 2) {
        usage(argv[0]);
        return 0;
    }

    bool as_server = false;
    if ( 0 == strcmp(argv[1], "true")) {
        as_server = true;
    } else if (0 != strcmp(argv[1], "false")) {
        usage(argv[0]);
        return 0;
    }

    // Init net
    g_send_fd = init_net(as_server);
    assert(g_send_fd > 0);

    // Init command thread.
    pthread_t th_command;
    int ret = pthread_create(&th_command, NULL, command_fun, NULL);
    exit_if_failed(ret, "Failed to create thread");

    // Init osip
    osip_t* osip = NULL;
    ret = osip_init(&osip);
    exit_if_failed(ret, "Failed to init osip");
    g_osip = osip;

    // set callbacks.
    // callback called when a SIP message must be sent.
    osip_set_cb_send_message(osip, &cb_send_message);
    // callback called when a SIP transaction is TERMINATED.
    osip_set_kill_transaction_callback(osip ,OSIP_ICT_KILL_TRANSACTION, &cb_xixt_kill_transaction);
    osip_set_kill_transaction_callback(osip ,OSIP_IST_KILL_TRANSACTION, &cb_xixt_kill_transaction);
    osip_set_kill_transaction_callback(osip ,OSIP_NICT_KILL_TRANSACTION, &cb_xixt_kill_transaction);
    osip_set_kill_transaction_callback(osip ,OSIP_NIST_KILL_TRANSACTION, &cb_xixt_kill_transaction);
    //callback called when the callback to send message have failed.
    osip_set_transport_error_callback(osip ,OSIP_ICT_TRANSPORT_ERROR, &cb_transport_error);
    osip_set_transport_error_callback(osip ,OSIP_IST_TRANSPORT_ERROR, &cb_transport_error);
    osip_set_transport_error_callback(osip ,OSIP_NICT_TRANSPORT_ERROR, &cb_transport_error);
    osip_set_transport_error_callback(osip ,OSIP_NIST_TRANSPORT_ERROR, &cb_transport_error);
    // callback called when a received answer has been accepted by the transaction.
    osip_set_message_callback(osip ,OSIP_ICT_STATUS_1XX_RECEIVED, &cb_rcv1xx);
    osip_set_message_callback(osip ,OSIP_ICT_STATUS_2XX_RECEIVED, &cb_rcv2xx);
    osip_set_message_callback(osip ,OSIP_ICT_STATUS_3XX_RECEIVED, &cb_rcv3xx);
    osip_set_message_callback(osip ,OSIP_ICT_STATUS_4XX_RECEIVED, &cb_rcv4xx);
    osip_set_message_callback(osip ,OSIP_ICT_STATUS_5XX_RECEIVED, &cb_rcv5xx);
    osip_set_message_callback(osip ,OSIP_ICT_STATUS_6XX_RECEIVED, &cb_rcv6xx);
    // callback called when a received answer has been accepted by the transaction.
    osip_set_message_callback(osip ,OSIP_NICT_STATUS_1XX_RECEIVED, &cb_rcv1xx);
    osip_set_message_callback(osip ,OSIP_NICT_STATUS_2XX_RECEIVED, &cb_rcv2xx);
    osip_set_message_callback(osip ,OSIP_NICT_STATUS_3XX_RECEIVED, &cb_rcv3xx);
    osip_set_message_callback(osip ,OSIP_NICT_STATUS_4XX_RECEIVED, &cb_rcv4xx);
    osip_set_message_callback(osip ,OSIP_NICT_STATUS_5XX_RECEIVED, &cb_rcv5xx);
    osip_set_message_callback(osip ,OSIP_NICT_STATUS_6XX_RECEIVED, &cb_rcv6xx);
    // callback called when a received request has been accepted by the transaction.
    osip_set_message_callback(osip ,OSIP_IST_INVITE_RECEIVED,       &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_IST_INVITE_RECEIVED_AGAIN, &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_IST_ACK_RECEIVED,          &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_IST_ACK_RECEIVED_AGAIN,    &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_REGISTER_RECEIVED,    &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_BYE_RECEIVED,         &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_CANCEL_RECEIVED,      &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_INFO_RECEIVED,        &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_OPTIONS_RECEIVED,     &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_SUBSCRIBE_RECEIVED,   &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_NOTIFY_RECEIVED,      &cb_rcvreq);
    osip_set_message_callback(osip ,OSIP_NIST_UNKNOWN_REQUEST_RECEIVED, &cb_rcvreq);

    // setup transport layer.
    osip_thread_create(0, transport_fun, osip);

    while(true)
    {       
        osip_ict_execute(osip);
        osip_ist_execute(osip);
        osip_nict_execute(osip);
        osip_nist_execute(osip);
        osip_timers_ict_execute(osip);
        osip_timers_ist_execute(osip);
        osip_timers_nict_execute(osip);
        osip_timers_nist_execute(osip);
        // This is not block. We need to block it. Use select.
    }

    pthread_join(th_command, NULL);
    
    cleanup_socket();
    return 0;
}

int cb_send_message(osip_transaction_t * tr, osip_message_t * sip, char *host, int port, int out_socket)
{
    char *msg;
    size_t msgLen;
    int ret = osip_message_to_str(sip, &msg, &msgLen);
    if (ret != 0) {
        std::cout << "Cannot get printable msg:";
        return -1;
    }
    
    std::stringstream ss;
    ss << "cb_send_message: " << tr->topvia->host << ':' << tr->topvia->port;

    write(g_send_fd, msg, msgLen);
    return OSIP_SUCCESS;
}

void cb_xixt_kill_transaction(int type, osip_transaction_t * tr)
{
    std::cout << __FUNCTION__ << " ";
    switch (type) {
    case OSIP_ICT_KILL_TRANSACTION:
        std::cout << "OSIP_ICT_KILL_TRANSACTION ";
        break;
    case OSIP_IST_KILL_TRANSACTION:
        std::cout << "OSIP_IST_KILL_TRANSACTION ";
        break;
    case OSIP_NICT_KILL_TRANSACTION:
        std::cout << "OSIP_NICT_KILL_TRANSACTION ";
        break;
    case OSIP_NIST_KILL_TRANSACTION:
        std::cout << "OSIP_NIST_KILL_TRANSACTION ";
        break;
    default:
        assert(false); // not reachable.
        break;
    }

    std::cout << ", transactionid=" << tr->transactionid << std::endl;

    // First, remove transacation.
    osip_t *osip = (osip_t*) osip_transaction_get_reserved1 (tr);
    assert(osip == g_osip);
    int i = osip_remove_transaction(osip, tr);
    if (i != 0) {
        std::cout << "osip_remove_transacation failed:" << i
                << ", transcationid=" << tr->transactionid << std::endl;
    }

}

void cb_transport_error(int type, osip_transaction_t * tr, int error)
{
    std::cout << __FUNCTION__ << std::endl;
}

void cb_rcv1xx(int type, osip_transaction_t * tr, osip_message_t * sip)
{
    std::cout << __FUNCTION__ << std::endl;
}

void cb_rcv2xx(int type, osip_transaction_t * tr, osip_message_t * sip)
{
    std::cout << __FUNCTION__ << std::endl;
}

void cb_rcv3xx(int type, osip_transaction_t * tr, osip_message_t * sip)
{
    std::cout << __FUNCTION__ << std::endl;
}

void cb_rcv4xx(int type, osip_transaction_t * tr, osip_message_t * sip)
{
    std::cout << __FUNCTION__ << std::endl;
}

void cb_rcv5xx(int type, osip_transaction_t * tr, osip_message_t * sip)
{
    std::cout << __FUNCTION__ << std::endl;
}

void cb_rcv6xx(int type, osip_transaction_t * tr, osip_message_t * sip)
{
    std::cout << __FUNCTION__ << std::endl;
}

void cb_rcvreq(int type, osip_transaction_t * tr, osip_message_t * sip)
{
    std::cout << __FUNCTION__ << std::endl;
}