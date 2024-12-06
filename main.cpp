#include <iio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <assert.h>
#include <chrono>

void config_phy(const struct iio_context *ctx);
int receive(const struct iio_context *ctx, long to_run);

bool run = true;

static void handle_sig(int sig)
{
	printf("Waiting for process to finish... Got signal %d\n", sig);
	run = false;
}


 
int main (int argc, char **argv)
{
//	struct iio_context *ctx;
//	struct iio_device *phy;
//	struct iio_channel *voltage0, *altvoltage0;
	long seconds_to_run = 0;
	if(argc == 2){
		//just one argument
		seconds_to_run = atoi(argv[1]);
	}

	int err = 0;

	signal(SIGINT, handle_sig);
 
	const auto ctx = iio_create_context_from_uri("ip:192.168.2.1");
	assert(ctx != nullptr);

	config_phy(ctx);
	receive(ctx, seconds_to_run);

	iio_context_destroy(ctx);

	return 0;
}

//Configure physical device3
void config_phy(const struct iio_context *ctx){
	const auto phy = iio_context_find_device(ctx, "ad9361-phy"); //iio:device в iio_info
	assert(phy != nullptr);
	const auto altvoltage0 = iio_device_find_channel(phy, "altvoltage0", true);// RX_LO (output)
	assert(altvoltage0 != nullptr);
	const auto voltage0 = iio_device_find_channel(phy, "voltage0", false); // (input)
	assert(voltage0 != nullptr);
	int err = 0;
	// Названия атрибутов берём также из вывода iio_info
	err = iio_channel_attr_write_longlong(
		altvoltage0,
		"frequency",
		1575420000);
	assert(err == 0);
 
	err = iio_channel_attr_write_longlong(
		voltage0,
		"rf_bandwidth",
		2600000);
	assert(err == 0);

	err = iio_channel_attr_write_longlong(
		voltage0,
		"sampling_frequency",
		3000000);
	assert(err == 0);
 
	err = iio_channel_attr_write(
		voltage0,
		"rf_port_select",
		"A_BALANCED");
	assert(err == 11);//bytes written

	err = iio_channel_attr_write(
		voltage0,
		"gain_control_mode",
		"manual");
	assert(err == 7);//bytes written

	err = iio_channel_attr_write_longlong(
		voltage0,
		"hardwaregain",
		71);
	assert(err == 0);

	err = iio_device_attr_write(
		phy,
		"ensm_mode",
		"tdd");
	assert(err == 4);

	err = iio_device_debug_attr_write_bool(
		phy,
		"adi,xo-disable-use-ext-refclk-enable",
		false);
	assert(err == 0);

	err = iio_device_debug_attr_write_bool(
		phy,
		"adi,external-rx-lo-enable",
		true);
	assert(err == 0);

	err = iio_device_debug_attr_write_bool(
		phy,
		"adi,external-tx-lo-enable",
		true);
	assert(err == 0);
}

int receive(const struct iio_context *ctx, long to_run)
{
	std::ofstream out_data("samples.dat", std::ios::out | std::ios::binary);
	if(!out_data){
		std::cout << "Cannot open file samples.dat" << std::endl;
		return 1;
	}

//	struct iio_device *dev;
//	struct iio_channel *rx0_i, *rx0_q;
//	struct iio_buffer *rxbuf;
 
	const auto dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
	assert(dev != nullptr);
	const auto rx0_i = iio_device_find_channel(dev, "voltage0", 0);
	assert(dev != nullptr);
	const auto rx0_q = iio_device_find_channel(dev, "voltage1", 0);
	assert(dev != nullptr);
 
	iio_channel_enable(rx0_i);
	iio_channel_enable(rx0_q);
 
	const auto rxbuf = iio_device_create_buffer(dev, 1024*1024, false);
	assert(rxbuf != nullptr);

	const auto stop_time = std::chrono::system_clock::now() + std::chrono::seconds(to_run);
	while (run && std::chrono::system_clock::now() < stop_time) {
		char *p_dat, *p_end, *t_dat;
		ptrdiff_t p_inc;
 
		auto refilled = iio_buffer_refill(rxbuf);
		assert(refilled >= 0);
 
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


