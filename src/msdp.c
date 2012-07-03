#include "tfconfig.h"
#include "port.h"
#include "tf.h"
#include "msdp.h"
#include "telopt.h"
#include "search.h"
#include "world.h"
#include "history.h"
#include "tfio.h"

struct _msdp_commands {
	const char * cmd;
	int id;
	int flags;
};

static const struct _msdp_commands msdp_commands[] = {
	{ "LIST", 1, 0 },
	{ "GET", 2, 0 },
	{ "REPORT", 3, 0 },
};



struct Value *handle_msdp_command(String * args, int offset) {
	const char buf[] = { TN_MSDP, MSDP_VAR, 'N', 'A', 'M', 'E', MSDP_VAL, 'D', 'A', 'N', TN_IAC, TN_SE };
	recv_msdp_sb(buf, strlen(buf));
	return newint(0);
}

conString * msdp_encode(const char * in) {
	STATIC_BUFFER(buf);
	Stringtrunc(buf, 0);
	Stringadd(buf, TN_IAC);
	Stringadd(buf, TN_SB);
	Stringadd(buf, TN_MSDP);



	return CS(buf);
}


int update_mdsp_value(const char * name, const char * value) {
}

int recv_msdp_sb(const char *p, int olen) {
	STATIC_BUFFER(path);
	STATIC_BUFFER(value);
	STATIC_BUFFER(test);
	const char * pend=p+olen;
	int ret=0;
	int namedepth=0;

	Stringcat(test, "% MSDP recv ");

	/* start with TN_MSDP */
	if (*p && *p == TN_MSDP) {
		int len;
		*p++;
		const char *c;
		int t;

		while (p<pend) {
			switch(*p) {
			case MSDP_VAR:
				c=p+1;
				while(c<pend) {
					if (*c<6 || *c==TN_IAC)
						break;
					c++;
				}
				Stringtrunc(path, namedepth);
				Stringncpy(path, p+1, c-p-1);
				SStringcat(test, CS(path));
				Stringadd(test, '=');
				p=c;
				break;


			case MSDP_VAL:
				c=p+1;
				while(c<pend) {
					if (*c<6 || *c==TN_IAC)
						break;
					c++;
				}
				Stringtrunc(value, namedepth);
				Stringncpy(value, p+1, c-p-1);
				SStringcat(test, CS(value));
				Stringcat(test, ", ");
				p=c;
				break;
			case MSDP_TABLE_OPEN:
			case MSDP_TABLE_CLOSE:
			case MSDP_ARRAY_OPEN:
			case MSDP_ARRAY_CLOSE:
				break; // unhandled
			case TN_IAC:
				if (p[1] == TN_SE)
					p=pend;
				else
					p++;
				break;
			default:
				p++;
			}
		}
	} else {
		ret=-1;
	}

	tfputline(CS(test), tfout);

	return ret;
}

