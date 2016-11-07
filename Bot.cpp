/**
 * Bot.cpp - Toby client library
 * Created by Gabriel Garcia, November 7, 2016.
 */

#include "Arduino.h"
#include "Bot.h"

Bot::Bot(String botId, String botSk) {
  _id = botId;
  _sk = botSk;
}


