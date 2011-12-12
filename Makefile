CFLAGS=
LDFLAGS=-lpng

stencilizer: stencilizer.c
	cc $(CFLAGS) -o stencilizer stencilizer.c $(LDFLAGS)

clean:
	rm -f *~ stencilizer

install: stencilizer
	install -c -m 555 stencilizer /usr/local/bin/stencilizer
