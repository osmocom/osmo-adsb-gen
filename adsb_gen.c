/* (C) 2014 by Harald Welte <laforge@gnumonks.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <osmocom/core/bits.h>
#include <osmocom/core/utils.h>

#define PREAMBLE_LEN	8
#define	SHORT_LEN	56
#define LONG_LEN	112

#define OUTBUF_SIZE	((LONG_LEN+PREAMBLE_LEN)*2)

#define SPACE	0xff

static const ubit_t preamble[PREAMBLE_LEN] = { 1, 1, SPACE, 0, 0, SPACE, SPACE, SPACE };

/* encode Pulse Position Modulation */
static int ppm_encode(ubit_t *out, const ubit_t *in, unsigned int num_in)
{
	int i;
	int j = 0;

	for (i = 0; i < num_in; i++) {
		switch (in[i]) {
		case 0:
			out[j++] = 0;
			out[j++] = 1;
			break;
		case 1:
			out[j++] = 1;
			out[j++] = 0;
			break;
		case SPACE:
			out[j++] = 0;
			out[j++] = 0;
			break;
		default:
			return -1;
		}
	}
	return 0;
}

int modes_encode_from_bin(ubit_t *out, const pbit_t *bin, unsigned int num_bits)
{
	ubit_t bitbuf[PREAMBLE_LEN+LONG_LEN];
	int rc, out_len;

	printf("from_bin(num_bits=%u)\n", num_bits);

	/* assemble the un-encoded bits of the entire message */
	memcpy(bitbuf, preamble, sizeof(preamble));
	rc = osmo_pbit2ubit(bitbuf+sizeof(preamble), bin, num_bits);

	printf("sym=%s\n", osmo_ubit_dump(bitbuf, num_bits+sizeof(preamble)));

	/* encode using pulse-position modulation */
	rc = ppm_encode(out, bitbuf, num_bits+sizeof(preamble));
	if (rc < 0) {
		fprintf(stderr, "Error during ppm_encode\n");
		return rc;
	}

	out_len = 2*(num_bits + sizeof(preamble));

	printf("out=%s\n", osmo_ubit_dump(out, out_len));

	return out_len;
}

int modes_encode_from_ascii(ubit_t *out, const char *in)
{
	char ascbuf[LONG_LEN*2];
	ubit_t binbuf[LONG_LEN];
	int str_len = strlen(in);
	int num_bytes = (str_len - 2) / 2;
	int rc;

	/* sanity checking */
	if (str_len > LONG_LEN*2 + 2) {
		fprintf(stderr, "String too long!\n");
		return -1;
	}

	if (in[0] != '*' || in[strlen(in)-1] != ';') {
		fprintf(stderr, "string `%s' not in correct format\n", in);
		return -2;
	}

	/* copy string, skipping heading '*' and trailing ';' */
	memset(ascbuf, 0, sizeof(ascbuf));
	strncpy(ascbuf, in+1, str_len-2);

	/* convert from hex to binary */
	rc = osmo_hexparse(ascbuf, binbuf, sizeof(binbuf));
	if (rc < 0) {
		fprintf(stderr, "error during hexparse of `%s'\n", ascbuf);
		return rc;
	}

	/* encode the actual binary */
	return modes_encode_from_bin(out, binbuf, num_bytes*8);
}

static int outfd;

/* write a frame to the output file */
static int put_frame(const ubit_t *frame, unsigned int num_bits)
{
	return write(outfd, frame, num_bits);
}

/* write a pause to the output file (given duration in usec) */
static int put_pause(unsigned int usec)
{
	uint8_t buf[100000];
	unsigned int i, num_samples = usec * 2;
	unsigned int written = 0;

	memset(buf, 0, sizeof(buf));

	for (i = 0; i < num_samples; i += written) {
		unsigned int pending = num_samples - i;
		int rc;

		written = pending;
		if (written > sizeof(buf))
			written = sizeof(buf);

		rc = write(outfd, buf, written);
		if (rc < written)
			return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int rc;
	char line[255];
	uint8_t outbuf[OUTBUF_SIZE];
	char *infname, *outfname;
	FILE *infile;
	int opt, pause_usec = 1000000;

	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
		case 'p':
			pause_usec = atoi(optarg);
			break;
		default:
			exit(2);
		}
	}

	if (argc <= optind+1) {
		fprintf(stderr, "Usage: %s [-p pause_usec] <infname> <outfname>\n", argv[0]);
		exit(2);
	}

	infname = argv[optind];
	outfname = argv[optind+1];

	infile = fopen(infname, "r");
	if (!infile)
		exit(2);

	outfd = open(outfname, O_CREAT|O_WRONLY|O_TRUNC, 0660);
	if (outfd < 0)
		exit(2);

	while (fgets(line, sizeof(line), infile)) {
		if (!strlen(line))
			continue;
		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';
		printf("IN: %s\n", line);

		rc = modes_encode_from_ascii(outbuf, line);
		if (rc < 0)
			exit(-rc);
		put_frame(outbuf, rc);

		put_pause(pause_usec);
	}

	fclose(infile);
	close(outfd);

	exit(0);
}
