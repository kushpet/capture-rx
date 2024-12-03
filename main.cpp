#include <iio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <signal.h>

int receive(struct iio_context *ctx);

bool run = true;

static void handle_sig(int sig)
{
	printf("Waiting for process to finish... Got signal %d\n", sig);
	run = false;
}


 
int main (int argc, char **argv)
{
	struct iio_context *ctx;
	struct iio_device *phy;

	signal(SIGINT, handle_sig);
 
	ctx = iio_create_context_from_uri("ip:192.168.2.1");
 
	phy = iio_context_find_device(ctx, "ad9361-phy"); //iio:device в iio_info
	// Названия атрибутов берём также из вывода iio_info
	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "altvoltage0", true), // RX_LO (output)
		"frequency",
		1575420000);
 
	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "voltage0", false), // (input)
		"rf_bandwidth",
		2600000);

	iio_channel_attr_write_longlong(
		iio_device_find_channel(phy, "voltage0", false), // (input)
		"sampling_frequency",
		3000000);
 
	iio_channel_attr_write(
		iio_device_find_channel(phy, "voltage0", false), // (input)
		"rf_port_select",
		"A_BALANCED");

	iio_channel_attr_write(
		iio_device_find_channel(phy, "voltage0", false), // (input)
		"gain_control_mode",
		"manual");

	iio_channel_attr_write_double(
		iio_device_find_channel(phy, "voltage0", false), // (input)
		"hardwaregain",
		71.0);

	receive(ctx);
 
	iio_context_destroy(ctx);
 
	return 0;
} 

int receive(struct iio_context *ctx)
{
	std::ofstream out_data("samples.dat", std::ios::out | std::ios::binary);
	if(!out_data){
		std::cout << "Cannot open file samples.dat" << std::endl;
		return 1;
	}

	struct iio_device *dev;
	struct iio_channel *rx0_i, *rx0_q;
	struct iio_buffer *rxbuf;
 
	dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
 
	rx0_i = iio_device_find_channel(dev, "voltage0", 0);
	rx0_q = iio_device_find_channel(dev, "voltage1", 0);
 
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);
 
	rxbuf = iio_device_create_buffer(dev, 1024*1024, false);
	if (!rxbuf) {
		std::cerr << "Could not create RX buffer";
		std::exit(-1);
	}
 
	while (run) {
		char *p_dat, *p_end, *t_dat;
		ptrdiff_t p_inc;
 
		iio_buffer_refill(rxbuf);
 
		p_inc = iio_buffer_step(rxbuf);
		p_end = (char*)iio_buffer_end(rxbuf);

		p_dat = (char*)iio_buffer_first(rxbuf, rx0_i);

		out_data.write(p_dat, p_end - p_dat);
 
//		for (p_dat = (char*)iio_buffer_first(rxbuf, rx0_i); p_dat < p_end; p_dat += p_inc, t_dat += p_inc) {
//			const int16_t i = ((int16_t*)p_dat)[0]; // Real (I)
//			const int16_t q = ((int16_t*)p_dat)[1]; // Imag (Q)
 
			/* Process here */
 
//		}
	}
	out_data.close();

	iio_buffer_destroy(rxbuf);
 
	return 0;
}


