/**
 * conf.h
 *
 * Convenience header for pre-specifying connection parameters for
 * testing.
 */

#define USE_PRIMARY 1
#define USE_ALT_A   1


#if USE_PRIMARY

#define REGISTRAR "ekiga.net"
#define STUN      "stun.ekiga.net"
#define ACCOUNT   ""
#define PASSWORD  ""
#define DEST      "sip:500@ekiga.net"

#elif USE_ALT_A

#define REGISTRAR "sip.diamondcard.us"
#define STUN      "stun.ekiga.net"
#define ACCOUNT   ""
#define PASSWORD  ""
#define DEST      "sip:###@sip.diamondcard.us"

#endif