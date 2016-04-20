#include "config.h"
#include "proc.h"
#include "sys.h"
#include "utils.h"
#include "gw_control.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>


static config cfg;
static sys_t sys;
static scene *sce;
static message *msg;
static message msg_decode;
static buffer_ring_byte bytes;

int main(int argn, char* argv[]){
	int m;
	int val;
	int len;
	znode_install *install;
	scene *sce;
	char buff[5000];

	get_config(&cfg);
	sys_init(&sys);

	buffer_ring_byte_create(&bytes, 5000); //create ring buffer given size len
	for (m = 0 ; m < ZNET_SIZE ; m ++){
		val = m + 30;
		install = &(sys.znode_install_list[m]);
		bzero(install, sizeof(znode_install));
		memcpy (install->id, &(val), sizeof(int));
		install->type = 4;
		memcpy(install->name, "switch4", 7*sizeof(char));
		memcpy(install->pos, "outside", 7*sizeof(char));
		install->posType = 2;
	}

	sys_update_dev_install(&sys, "/home/liulingfeng/install");

	for (m = 0 ; m < ZNET_SIZE ; m ++){
		install = &(sys.znode_install_list[m]);
		bzero(install, sizeof(znode_install));
	}

	sys_get_dev_install(&sys, "/home/liulingfeng/install");

	for (m = 0 ; m < MAX_SCENE_NUM ; m ++){
		val = m + 30;
		sce = &(sys.sces[m]);
		bzero(sce, sizeof(scene));
		memcpy (sce->host_mac, &(val), sizeof(int));
		memcpy (sce->host_id_major, &(val), sizeof(int));
		memcpy (sce->host_id_minor, &(val), sizeof(int));
		sce->scene_type = 2;
		memcpy(sce->scene_name, "scene01", 7*sizeof(char));
		sce->trigger_num = 1;
		sce->item_num = 1;
		sce->trigger = calloc(1, sizeof(scene_item));
		memcpy(sce->trigger[0].id, "trigger", 7*sizeof(char));

		sce->item = calloc(1, sizeof(scene_item));
		memcpy(sce->item[0].id, "item001", 7*sizeof(char));
	}

	sys_update_scene(&sys, "/home/liulingfeng/scene");

	for (m = 0 ; m < ZNET_SIZE ; m ++){
		install = &(sys.znode_install_list[m]);
		bzero(install, sizeof(znode_install));
	}

	sys_get_scene(&sys, "/home/liulingfeng/scene");

	sce = &(sys.sces[0]);

	msg = message_create_scene(sys.id, sce);

	msg->data_type = DATA_SET_SCENE;



	len = 96+sce->trigger_num*16+sce->item_num*16;

	val = message2bytes(msg, buff); //return the length of the byte

	buffer_ring_byte_put(&bytes, buff, val); //put data into the buffer
	val = bytes2message(&bytes, &msg_decode);
	for (m = 0; m < len; m ++){
		printf("%x ", msg_decode.data[m] & 0x00ff);
	}
	printf("\n");

	for (m = 0; m < val; m ++){
		printf("%x ", buff[m] & 0x00ff);
	}
}
