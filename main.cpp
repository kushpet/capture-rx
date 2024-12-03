#include <iio.h>
#include <cstdlib>
#include <iostream>

int receive(struct iio_context *ctx);
 
int main (int argc, char **argv)
{
	struct iio_context *ctx;
	struct iio_device *phy;
 
	ctx = iio_create_context_from_uri("ip:192.168.2.1");
 
	phy = iio_context_find_device(ctx, "ad9361-phy");
 
	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "altvoltage0", true),
		"frequency",
		2400000000); /* RX LO frequency 2.4GHz */
 
	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "voltage0", false),
		"sampling_frequency",
		5000000); /* RX baseband rate 5 MSPS */
 
	receive(ctx);
 
	iio_context_destroy(ctx);
 
	return 0;
} 

int receive(struct iio_context *ctx)
{
	struct iio_device *dev;
	struct iio_channel *rx0_i, *rx0_q;
	struct iio_buffer *rxbuf;
 
	dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
 
	rx0_i = iio_device_find_channel(dev, "voltage0", 0);
	rx0_q = iio_device_find_channel(dev, "voltage1", 0);
 
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);
 
	rxbuf = iio_device_create_buffer(dev, 4096, false);
	if (!rxbuf) {
		std::cerr << "Could not create RX buffer";
		std::exit(-1);
	}
 
	while (true) {
		void *p_dat, *p_end, *t_dat;
		ptrdiff_t p_inc;
 
		iio_buffer_refill(rxbuf);
 
		p_inc = iio_buffer_step(rxbuf);
		p_end = iio_buffer_end(rxbuf);
 
		for (p_dat = iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc, t_dat += p_inc) {
			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
 
			/* Process here */
 
		}
	}
 
	iio_buffer_destroy(rxbuf);
 
}


