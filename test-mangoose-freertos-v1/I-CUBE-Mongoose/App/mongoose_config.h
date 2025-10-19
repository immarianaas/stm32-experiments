
#define MG_ARCH MG_ARCH_FREERTOS

#define MG_ENABLE_TCPIP_DRIVER_INIT 0
#define MG_ENABLE_TCPIP 0
#define MG_ENABLE_LWIP 1

#define MG_ENABLE_PACKED_FS  1

#define MG_STMPACK_TLS 1

// Translate to Mongoose macros
#if MG_STMPACK_TLS == 0
#define MG_TLS MG_TLS_NONE
#elif MG_STMPACK_TLS == 1
#define MG_TLS MG_TLS_BUILTIN
#elif MG_STMPACK_TLS == 2
#define MG_TLS MG_TLS_MBEDTLS
#elif MG_STMPACK_TLS == 3
#define MG_TLS MG_TLS_WOLFSSL
#endif

// See https://mongoose.ws/documentation/#build-options
