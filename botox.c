#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <tox/tox.h>

struct node {
  char *host;
  int port;
  char key[TOX_PUBLIC_KEY_SIZE * 2 + 1]
};

struct node nodes[] = {
  {"85.143.221.42",                      33445, "DA4E4ED4B697F2E9B000EEFE3A34B554ACD3F45F5C96EAEA2516DD7FF9AF7B43"},
  {"2a04:ac00:1:9f00:5054:ff:fe01:becd", 33445, "DA4E4ED4B697F2E9B000EEFE3A34B554ACD3F45F5C96EAEA2516DD7FF9AF7B43"},
  {"78.46.73.141",                       33445, "02807CF4F8BB8FB390CC3794BDF1E8449E9A8392C5D3F2200019DA9F1E812E46"},
  {"2a01:4f8:120:4091::3",               33445, "02807CF4F8BB8FB390CC3794BDF1E8449E9A8392C5D3F2200019DA9F1E812E46"},
  {"tox.initramfs.io",                   33445, "3F0A45A268367C1BEA652F258C85F4A66DA76BCAA667A49E770BCC4917AB6A25"},
  {"tox2.abilinski.com",                 33445, "7A6098B590BDC73F9723FC59F82B3F9085A64D1B213AAF8E610FD351930D052D"},
  {"205.185.115.131",                       53, "3091C6BEB2A993F1C6300C16549FABA67098FF3D62C6D253828B531470B53D68"},
  {"tox.kurnevsky.net",                  33445, "82EF82BA33445A1F91A7DB27189ECFC0C013E06E3DA71F588ED692BED625EC23"}
};
int nodes_count = 8;

void handle_friend_request(Tox *tox, const uint8_t *public_key, const uint8_t *message, size_t length, void *user_data)
{
  TOX_ERR_FRIEND_ADD err_add;
  tox_friend_add_norequest(tox, public_key, &err_add);
  if (err_add != TOX_ERR_FRIEND_ADD_OK)
  {
    printf("ERROR: failed to accept friend request\n");
    return;
  }

  printf("INFO: friend request accepted\n", public_key);
}

void handle_friend_message(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *user_data)
{
  TOX_ERR_FRIEND_SEND_MESSAGE err_send;
  tox_friend_send_message(tox, friend_number, type, message, length, &err_send);
  if (err_send != TOX_ERR_FRIEND_SEND_MESSAGE_OK)
  {
    printf("ERROR: failed to send message\n");
    return;
  }

  printf("INFO: message sent: %s\n", message);
}

void handle_self_connection_status(Tox *tox, Tox_Connection status, void *user_data)
{
  switch (status)
  {
  case TOX_CONNECTION_NONE:
    printf("INFO: Offline\n");
    break;
  case TOX_CONNECTION_UDP:
    printf("INFO: Online (UDP)\n");
    break;
  case TOX_CONNECTION_TCP:
    printf("INFO: Online (TCP)\n");
    break;
  }
}

void key_hex2bin(char *hex, uint8_t *bin) {
  for (int i = 0; i < TOX_PUBLIC_KEY_SIZE; i++)
    sscanf(&hex[i*2], "%2hhx", &bin[i]);
}

int run()
{
  TOX_ERR_NEW err_new;

  Tox *tox = tox_new(NULL, &err_new);
  if (err_new != TOX_ERR_NEW_OK)
  {
    printf("ERROR: failed to create tox instance\n");
    return 1;
  }

  char *name = "Botox";
  TOX_ERR_SET_INFO err_info;
  tox_self_set_name(tox, name, strlen(name), &err_info);
  if (err_info != TOX_ERR_SET_INFO_OK)
  {
    printf("ERROR: failed to set name");
    return 1;
  }

  char *msg = "ou man cafe eh bom dimas";
  tox_self_set_status_message(tox, msg, strlen(msg), &err_info);
  if (err_info != TOX_ERR_SET_INFO_OK)
  {
    printf("ERROR: failed to change status message");
    return 1;
  }

  uint8_t addr[TOX_ADDRESS_SIZE];
  tox_self_get_address(tox, addr);


  for (int i = 0; i < nodes_count; i++) {
    struct node *n = &nodes[i];
    char key[TOX_PUBLIC_KEY_SIZE];
    key_hex2bin(n->key, key);

    TOX_ERR_BOOTSTRAP err;
    tox_bootstrap(tox, n->host, n->port, key, &err);
    if (err != TOX_ERR_BOOTSTRAP_OK) {
      printf("WARN: failed to bootstrap (%s, %d, %s)\n", n->host, n->port, n->key);
      continue;
    }
    printf("INFO: bootstrap succeeded (%s, %d, %s)\n", n->host, n->port, n->key);
  }

  tox_callback_self_connection_status(tox, handle_self_connection_status);
  tox_callback_friend_request(tox, handle_friend_request);
  tox_callback_friend_message(tox, handle_friend_message);

  printf("INFO: tox ID: ");
  for (int i = 0; i < TOX_ADDRESS_SIZE; i++)
    printf("%02X", addr[i]);
  printf("\n");
  printf("INFO: connecting...\n");

  for (;;)
  {
    tox_iterate(tox, NULL);
    usleep(100000 * tox_iteration_interval(tox));
  }

  return 0;
}

int main()
{
  exit(run());
}