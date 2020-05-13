#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

typedef struct _item
{
	char type;
	void *var;
} item;

/**
 * Connect to the DBUS bus and send a broadcast signal
 */
void sendsignal(char* sigvalue)
{
	DBusMessage* msg;
	DBusMessageIter args;
	DBusConnection* conn;
	DBusError err;
	int ret;
	dbus_uint32_t serial = 0;

	printf("Sending signal with value %s\n", sigvalue);

	// initialise the error value
	dbus_error_init(&err);

	// connect to the DBUS system bus, and check for errors
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Connection Error (%s)\n", err.message); 
		dbus_error_free(&err); 
	}
	if (NULL == conn) { 
		exit(1); 
	}

	// register our name on the bus, and check for errors
	ret = dbus_bus_request_name(conn, "test.signal.source", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Name Error (%s)\n", err.message); 
		dbus_error_free(&err); 
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
		exit(1);
	}

	// create a signal & check for errors 
	msg = dbus_message_new_signal("/test/signal/Object", // object name of the signal
											"test.signal.Type", // interface name of the signal
											"Test"); // name of the signal
	if (NULL == msg) 
	{ 
		fprintf(stderr, "Message Null\n"); 
		exit(1); 
	}

	// append arguments onto signal
	dbus_message_iter_init_append(msg, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &sigvalue)) {
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}

	// send the message and flush the connection
	if (!dbus_connection_send(conn, msg, &serial)) {
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}
	dbus_connection_flush(conn);
	
	printf("Signal Sent\n");
	
	// free the message and close the connection
	dbus_message_unref(msg);
	dbus_connection_close(conn);
}

/**
 * Call a method on a remote object
 */
void query(char* param) 
{
	DBusMessage* msg;
	DBusMessageIter args;
	DBusConnection* conn;
	DBusError err;
	DBusPendingCall* pending;
	int ret;
	dbus_uint32_t stat;
	dbus_uint32_t level;

	printf("Calling remote method with %s\n", param);

	// initialiset the errors
	dbus_error_init(&err);

	// connect to the system bus and check for errors
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Connection Error (%s)\n", err.message); 
		dbus_error_free(&err);
	}
	if (NULL == conn) { 
		exit(1); 
	}

	// request our name on the bus
	ret = dbus_bus_request_name(conn, "test.method.caller", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Name Error (%s)\n", err.message); 
		dbus_error_free(&err);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
		exit(1);
	}

	// create a new method call and check for errors
	msg = dbus_message_new_method_call("test.method.server", // target for the method call
												  "/test/method/Object", // object to call on
												  "test.method.Type", // interface to call on
												  "Method"); // method name
	if (NULL == msg) { 
		fprintf(stderr, "Message Null\n");
		exit(1);
	}

	// append arguments
	dbus_message_iter_init_append(msg, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param)) {
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}
	
	// send message and get a handle for a reply
	if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}
	if (NULL == pending) { 
		fprintf(stderr, "Pending Call Null\n"); 
		exit(1); 
	}
	dbus_connection_flush(conn);
	
	printf("Request Sent\n");
	fprintf(stderr, "dbus_connection_send_with_reply(): "); getchar();
	
	// free message
	dbus_message_unref(msg);
	
	// block until we recieve a reply
	dbus_pending_call_block(pending);

	// get the reply message
	msg = dbus_pending_call_steal_reply(pending);
	if (NULL == msg) {
		fprintf(stderr, "Reply Null\n"); 
		exit(1); 
	}
	// free the pending message handle
	dbus_pending_call_unref(pending);

	// read the parameters
	if (!dbus_message_iter_init(msg, &args))
		fprintf(stderr, "Message has no arguments!\n"); 
	else if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args)) 
		fprintf(stderr, "Argument is not boolean!\n"); 
	else
		dbus_message_iter_get_basic(&args, &stat);

	if (!dbus_message_iter_next(&args))
		fprintf(stderr, "Message has too few arguments!\n"); 
	else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args)) 
		fprintf(stderr, "Argument is not int!\n"); 
	else
		dbus_message_iter_get_basic(&args, &level);

	printf("Got Reply: %d, %d\n", stat, level);
	
	// free reply and close connection
	dbus_message_unref(msg);	
	//dbus_connection_close(conn);
}

void reply_to_method_call(DBusMessage* msg, DBusConnection* conn)
{
	DBusMessage* reply;
	DBusMessageIter args;
	dbus_uint32_t stat = true;
	dbus_uint32_t level = 21614;
	dbus_uint32_t serial = 0;
	char* param = "";

	// read the arguments
	if (!dbus_message_iter_init(msg, &args))
		fprintf(stderr, "Message has no arguments!\n"); 
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) 
		fprintf(stderr, "Argument is not string!\n"); 
	else 
		dbus_message_iter_get_basic(&args, &param);

	printf("Method called with %s\n", param);

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);
	printf("dbus_message_new_method_return reply := 0x%x\n", reply);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	printf("dbus_message_iter_init_append\n");
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &stat)) { 
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}
	printf("dbus_message_iter_append_basic\n");
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &level)) { 
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}

	printf("dbus_connection_send\n");
	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

/**
 * Server that exposes a method call and waits for it to be called
 */
void listen() 
{
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusConnection* conn;
	DBusError err;
	int ret;
	char* param;

	printf("Listening for method calls\n");

	// initialise the error
	dbus_error_init(&err);
	
	// connect to the bus and check for errors
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Connection Error (%s)\n", err.message); 
		dbus_error_free(&err); 
	}
	if (NULL == conn) {
		fprintf(stderr, "Connection Null\n"); 
		exit(1); 
	}
	
	// request our name on the bus and check for errors
	ret = dbus_bus_request_name(conn, "test.method.server", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Name Error (%s)\n", err.message); 
		dbus_error_free(&err);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
		fprintf(stderr, "Not Primary Owner (%d)\n", ret);
		exit(1); 
	}

	// loop, testing for new messages
	while (true) {
		// non blocking read of the next available message
		dbus_connection_read_write(conn, 0);
		msg = dbus_connection_pop_message(conn);

		// loop again if we haven't got a message
		if (NULL == msg) { 
			sleep(1); 
			continue; 
		}
		fprintf(stderr, "new msg := 0x%x\n", msg);
		
		// check this is a method call for the right interface & method
		if (dbus_message_is_method_call(msg, "test.method.Type", "Method")) {
	 fprintf(stderr, "reply_to_method_call\n");
			reply_to_method_call(msg, conn);
	 fprintf(stderr, "end\n");
		}//endif true

		// free the message
		dbus_message_unref(msg);
		fprintf(stderr, "\n");
	}

	// close the connection
	dbus_connection_close(conn);
}

/**
 * Listens for signals on the bus
 */
void receive()
{
	DBusMessage* msg;
	DBusMessageIter args;
	DBusConnection* conn;
	DBusError err;
	int ret;
	char* sigvalue;

	printf("Listening for signals\n");

	// initialise the errors
	dbus_error_init(&err);
	
	// connect to the bus and check for errors
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err); 
	}
	if (NULL == conn) { 
		exit(1);
	}
	
	// request our name on the bus and check for errors
	ret = dbus_bus_request_name(conn, "test.signal.sink", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Name Error (%s)\n", err.message);
		dbus_error_free(&err); 
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		exit(1);
	}

	// add a rule for which messages we want to see
	dbus_bus_add_match(conn, "type='signal',interface='test.signal.Type'", &err); // see signals from the given interface
	dbus_connection_flush(conn);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Match Error (%s)\n", err.message);
		exit(1); 
	}
	printf("Match rule sent\n");

	// loop listening for signals being emmitted
	while (true) {

		// non blocking read of the next available message
		dbus_connection_read_write(conn, 0);
		msg = dbus_connection_pop_message(conn);

		// loop again if we haven't read a message
		if (NULL == msg) { 
			sleep(1);
			continue;
		}

		// check if the message is a signal from the correct interface and with the correct name
		if (dbus_message_is_signal(msg, "test.signal.Type", "Test")) {
			
			// read the parameters
			if (!dbus_message_iter_init(msg, &args))
				fprintf(stderr, "Message Has No Parameters\n");
			else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) 
				fprintf(stderr, "Argument is not string!\n"); 
			else
				dbus_message_iter_get_basic(&args, &sigvalue);
			
			printf("Got Signal with value %s\n", sigvalue);
		}

		// free the message
		dbus_message_unref(msg);
	}
	// close the connection
	dbus_connection_close(conn);
}

GList *
callMethod(char *method, GList *listParams)
{
	DBusMessage* msg;
	DBusPendingCall* pending;
	DBusConnection* conn;
	DBusError err;
	DBusMessageIter args;
	void *param = NULL;
	dbus_uint64_t param_uint64;
	dbus_uint32_t param_uint32;
	dbus_int64_t param_int64;
	dbus_int32_t param_int32;
	//dbus_uint64_t param = atol(_param);

	// initialiset the errors
	dbus_error_init(&err);

	// connect to the system bus and check for errors
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Connection Error (%s)\n", err.message); 
		dbus_error_free(&err);
	}
	if (NULL == conn) { 
		exit(1); 
	}


	msg = dbus_message_new_method_call( "org.freedesktop.portal.Desktop", // target for the method call
												  "/org/freedesktop/portal/desktop", // object to call on
													"org.freedesktop.portal.LibUsb", // interface to call on
												  method); // method name
												  //"libusb_init"); // method name

	if (NULL == msg) { 
		fprintf(stderr, "Message Null\n");
		exit(1);
	}

	if(listParams && g_list_length(listParams)){
		dbus_message_iter_init_append(msg, &args);
		GList *tmp = g_list_first(listParams);

		while(tmp){
			item *p = tmp->data;

			fprintf(stderr, "type := %c, var := %ld\n", p->type, (*(unsigned long *)p->var));
			if (!dbus_message_iter_append_basic(&args, p->type, p->var)) { 
				fprintf(stderr, "Out Of Memory!\n"); 
				exit(1);
			}//endif true
			tmp = tmp->next;
		}//endwhile


		// clean list?

	}//endif true

	// send message and get a handle for a reply
	if (!dbus_connection_send_with_reply (conn, msg, &pending, 100)) { // -1 is default timeout
		fprintf(stderr, "Out Of Memory!\n"); 
		exit(1);
	}

	if (NULL == pending) { 
		fprintf(stderr, "Pending Call Null\n"); 
		exit(1); 
	}
	dbus_connection_flush(conn);

	// free message
	dbus_message_unref(msg);

	// block until we receive a reply
	dbus_pending_call_block(pending);
	
	// get the reply message
	msg = dbus_pending_call_steal_reply(pending);
	if (NULL == msg) {
		fprintf(stderr, "Reply Null\n"); 
		exit(1); 
	}

	GList *listResults = NULL;
	// read the parameters
	if (dbus_message_iter_init(msg, &args)){
		fprintf(stderr, "Got Reply: ");

		do {
			item *it = g_new(item, 1);
			it->type = dbus_message_iter_get_arg_type(&args); 
			fprintf(stderr, "(%c) ", it->type); 

			switch(it->type){
			case 'i':
				it->var = g_new(dbus_uint32_t, 1);
				break;
			case 't':
				it->var = g_new(dbus_uint64_t, 1);
				break;
			default :
				fprintf(stderr, "type error\n");
				continue;
			}//endswitch
		  	dbus_message_iter_get_basic(&args, it->var);
			switch(it->type){
			case 'i':
			case 't':
				fprintf(stderr, "%ld, ", (*(unsigned long *)it->var));
				break;
			case 's':
				fprintf(stderr, "%s, ", it->var);
				break;
			}//endswitch

			listResults = g_list_append(listResults, it);
		}while(dbus_message_iter_next(&args));

		fprintf(stderr, "\n");
	}//endif true
	else
		fprintf(stderr, "Message has no arguments!\n"); 

	// free reply and close connection
	dbus_message_unref(msg); 

	return(listResults);
}//end callMethod(char *args)


unsigned long
dbus_libusb_init(unsigned long context)
{
	GList *listResults = NULL;
	item *it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = context;

	listResults = callMethod("libusb_init", g_list_append(NULL, it));

	item *p = g_list_first(listResults)->data;
	context = (*(dbus_uint64_t*)p->var);

	return(context);
}//end dbus_libusb_init(unsigned long context)

void
dbus_libusb_exit(unsigned long context)
{
	GList *listResults = NULL;
	item *it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = context;

	listResults = callMethod("libusb_exit", g_list_append(NULL, it));

}//end dbus_libusb_exit(unsigned long context)

int
dbus_libusb_get_device_list(unsigned long context, unsigned long devs)
{
	GList *listResults = NULL, *listParams = NULL;
	item *it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = context;
	listParams = g_list_append(NULL, it);

	it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = devs;
	listParams = g_list_append(listParams, it);

	listResults = callMethod("libusb_get_device_list", listParams);

	item *p = g_list_first(listResults)->data;
	int cnt = (*(dbus_uint32_t*)p->var);

	return(cnt);
}//end dbus_libusb_get_device_list(unsigned long context, unsigned long devs)

void
dbus_libusb_free_device_list(unsigned long context, unsigned long devs)
{
	GList *listResults = NULL, *listParams = NULL;
	item *it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = context;
	listParams = g_list_append(NULL, it);

	it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = devs;
	listParams = g_list_append(listParams, it);

	listResults = callMethod("libusb_free_device_list", listParams);

	return;
}//end dbus_libusb_free_device_list(unsigned long context, unsigned long devs)

int main(int argc, char** argv)
{
	char* typeParams = "no param";
	GList *listParams = NULL, *listResults = NULL;

	//////////////////////////////////////////////////////////////////////////
	fprintf(stderr, "dbus_libusb_init(0) ... \n");
	dbus_uint64_t context = dbus_libusb_init(0);

	fprintf(stderr, "context := %ld.\n", context);
	fprintf(stderr, "\n");

	//////////////////////////////////////////////////////////////////////////
	fprintf(stderr, "dbus_libusb_get_device_list(%ld, 0) ... \n", context);
	int cnt = dbus_libusb_get_device_list(context, 0);
	fprintf(stderr, "cnt := %d\n", cnt);
	fprintf(stderr, "\n");

	//////////////////////////////////////////////////////////////////////////
	fprintf(stderr, "dbus_libusb_free_device_list(%ld, 0) ... \n", context);
	dbus_libusb_free_device_list(context, 0);
	fprintf(stderr, "\n");

	//////////////////////////////////////////////////////////////////////////
	fprintf(stderr, "dbus_libusb_exit(%ld) ... \n", context);
	dbus_libusb_exit(context);
	fprintf(stderr, "\n");

	/*
	//////////////////////////////////////////////////////////////////////////
	it = g_new(item, 1);
	it->type = 't';
	it->var = context;
	listParams = g_list_append(NULL, it);

	it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = 3;
	listParams = g_list_append(listParams, it);

	listResults = callMethod("libusb_get_device_list", listParams);

	it = (g_list_first(listResults)->data);
	unsigned int cnt = (*(unsigned int *)it->var);
	fprintf(stderr, "cnt := %d\n", cnt);
	fprintf(stderr, "\n");


	//////////////////////////////////////////////////////////////////////////
	it = g_new(item, 1);
	it->type = 't';
	it->var = context;
	listParams = g_list_append(NULL, it);

	it = g_new(item, 1);
	it->type = 't';
	it->var = g_new(dbus_uint64_t, 1);
	(*(dbus_uint64_t *)it->var) = 0;
	listParams = g_list_append(listParams, it);

	listResults = callMethod("libusb_free_device_list", listParams);

	fprintf(stderr, "\n");

	//////////////////////////////////////////////////////////////////////////
	it->type = 't';
	it->var = context;

	listResults = callMethod("libusb_exit", g_list_append(NULL, it));

	fprintf(stderr, "\n");
	*/












	return 0;
}
