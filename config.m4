PHP_ARG_ENABLE(flight, whether to enable flight support,
[  --enable-flight           Enable flight support])

if test "$PHP_FLIGHT" != "no"; then
   PHP_NEW_EXTENSION(flight,flight.c flight_app.c,$ext_shared)
fi
