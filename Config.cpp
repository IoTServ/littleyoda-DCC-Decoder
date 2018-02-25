/*
 * Config.cpp
 *
 *  Created on: 21.07.2017
 *      Author: sven
 */
#include "Arduino.h"
#include "Config.h"
#include "FS.h"
#include "Logger.h"
#include "ArduinoJson.h"
#include "GPIO.h"

#include "ActionTurnOut.h"
#include "ActionServo.h"
#include "ActionLed.h"
#include "ActionPWMOutput.h"
#include "ActionDFPlayerMP3.h"
#include "ActionDCCGeneration.h"
#include "ActionSUSIGeneration.h"
#include "ActionSendTurnoutCommand.h"

#include "Connectors.h"
#include "ConnectorLocoSpeed.h"
#include "ConnectorONOFF.h"
#include "ConnectorFunc2Value.h"
#include "ConnectorTurnout.h"
#include "ConnectorLights.h"
#include "ConnectorGPIO.h"
#include "Config.h"

#include "CmdReceiverDCC.h"
#include "CmdZentraleZ21.h"
#include "CmdReceiverZ21Wlan.h"
//#include "CmdReceiverESPNOW.h"

#include "WebserviceCommandLogger.h"
#include "WebserviceLog.h"
#include "WebserviceWifiScanner.h"

#include "ISettings.h"


Config::Config() {
}

Config::~Config() {
}


boolean Config::parse(Controller* controller, Webserver* web, String filename, boolean dryrun) {
	lowmemory = false;
	File configFile = SPIFFS.open(filename, "r");
	size_t size = configFile.size();
	Serial.println("Starting Parsing");
	char* buf = new char[size];
	configFile.readBytes(buf, size);
	jsmn_parser p;
	jsmn_init(&p);
	int r = jsmn_parse(&p, buf, size, NULL, 0);
	if (r < 0) {
		printf("Failed to parse JSON (Token): %d\r\n", r);
		return false;
	}
	jsmntok_t* t = new jsmntok_t[r];
	jsmn_init(&p);
	r = jsmn_parse(&p, buf, size, t, r);
	if (r < 0) {
		Serial.printf("Failed to parse JSON: %d\r\n", r);
		return false;
	}
	Serial.println("Ok2");
	Serial.flush();
	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\r\n");
		return 1;
	}

	for (int i = 1; i < r; i++) {
		Serial.println("Pos: " + String(i));
		printf("- User: %.*s\r\n", t[i].end-t[i].start,
				buf + t[i].start);
		//Serial.println("Start: " + String(t[i].start) + " " + String(t[i].end));
//		Serial.printf("Key: %.*s\r\n", t[i].end-t[i].start, buf.get() + t[i].start);
//		printf("  * %.*s\r\n", g->end - g->start, buf.get() + g->start);
//		if (jsoneq(buf.get(), &t[i], "user") == 0) {
//			/* We may use strndup() to fetch string value */
//			printf("- User: %.*s\r\n", t[i+1].end-t[i+1].start,
//					buf.get() + t[i+1].start);
//			i++;
//		} else if (jsoneq(buf.get(), &t[i], "admin") == 0) {
//			/* We may additionally check if the value is either "true" or "false" */
//			printf("- Admin: %.*s\r\n", t[i+1].end-t[i+1].start,
//					buf.get() + t[i+1].start);
//			i++;
//		} else if (jsoneq(buf.get(), &t[i], "uid") == 0) {
//			/* We may want to do strtol() here to get numeric value */
//			printf("- UID: %.*s\r\n", t[i+1].end-t[i+1].start,
//					buf.get() + t[i+1].start);
//			i++;
//		} else if (jsoneq(buf.get(), &t[i], "groups") == 0) {
//			int j;
//			printf("- Groups:\r\n");
//			if (t[i+1].type != JSMN_ARRAY) {
//				continue; /* We expect groups to be an array of strings */
//			}
//			for (j = 0; j < t[i+1].size; j++) {
//				jsmntok_t *g = &t[i+j+2];
//				printf("  * %.*s\r\n", g->end - g->start, buf.get() + g->start);
//			}
//			i += t[i+1].size + 1;
//		} else {
//			printf("Unexpected key: %.*s\r\n", t[i].end-t[i].start,
//					buf.get() + t[i].start);
//		}
	}

	//	DynamicJsonBuffer jsonBuffer;
	//	JsonObject& root = jsonBuffer.parseObject(configFile);
	//
	//	if (!root.success()) {
	//		Serial.println("Parsing failed!");
	//		return false;
	//	}
	//	Serial.println("Parsing ok!");
	//	Serial.println("MEM "  + String(ESP.getFreeHeap()) + " / Cfg-Setup");
	//	int version = root["version"].as<int>();
	//	if (version != 3) {
	//		Logger::getInstance()->addToLog("Ungültige Version: " + String(version));
	//		return false;
	//	}
	//	if (dryrun) {
	//		return true;
	//	}
	//
	//	JsonArray& cfg = root["cfg"];
	//	parseCfg(controller, web, cfg);
	//
	//	JsonArray& out = root["out"];
	//	parseOut(controller, web, out);
	//
	//	JsonArray& in = root["in"];
	//	parseIn(controller, web, in);
	//
	//	JsonArray& connector = root["connector"];
	//	parseConnector(controller, web, connector);
	//	configFile.close();
	//
	//	Logger::getInstance()->addToLog("JSON Parsing finish!");
	//	Serial.println("MEM "  + String(ESP.getFreeHeap()) + " / Cfg-Clear");
	//	jsonBuffer.clear();
	//	Serial.println("MEM "  + String(ESP.getFreeHeap()) + " / Cfg-End");
	//	if (lowmemory) {
	//		Logger::getInstance()->addToLog("Nicht genügend Speicher für das JSON-File!");
	//		return false;
	//	}
	return false;
}

void Config::parseOut(Controller* controller, Webserver* web, JsonArray& r1) {
	for (JsonArray::iterator it = r1.begin(); it != r1.end(); ++it) {
		if (ESP.getFreeHeap() < 1200) {
			lowmemory = true;
			break;
		}
		JsonObject& value = *it;
		const char* art = (const char*) value["m"];
		if (art == NULL) {
			//Logger::getInstance()->addToLog("Null from json");
			continue;
		}
		if (strcmp(art, "dccout") == 0) {
			Pin* gpioenable = new Pin(value["enable"].as<const char*>());
			int locoaddr = value["addr"].as<int>();
			int dccoutput = value["dccoutputaddr"].as<int>();
			ActionDCCGeneration* a = new ActionDCCGeneration(gpioenable, locoaddr, dccoutput);
			controller->registerNotify(a);
			controller->registerLoop(a);
			continue;
		}

		if (strcmp(art, "susiout") == 0) {
			int locoaddr = value["addr"].as<int>();
			ActionSUSIGeneration* a = new ActionSUSIGeneration(locoaddr);
			controller->registerNotify(a);
			controller->registerLoop(a);
			continue;
		}

		const char* id = (const char*) value["id"];
		if (id == NULL) {
			Logger::getInstance()->addToLog("ID is null");
			continue;
		}
		if (strcmp(art, "led") == 0) {
			Pin* ledgpio = new Pin(value["gpio"].as<const char*>());
			ActionLed* l = new ActionLed(ledgpio);
			l->setName(id);
			controller->registerSettings(l);
			controller->registerLoop(l);
		} else if (strcmp(art, "pwm") == 0) {
			int gpiopwm = GPIO.string2gpio(value["pwm"].as<const char*>());
			int gpiof = GPIO.string2gpio(value["forward"].as<const char*>());
			int gpior = GPIO.string2gpio(value["reverse"].as<const char*>());
			ISettings* a = new ActionPWMOutput(gpiopwm, gpiof, gpior);
			a->setName(id);
			controller->registerSettings(a);


		} else if (strcmp(art, "servo") == 0) {
			int gpioservo = GPIO.string2gpio(value["gpio"].as<const char*>());
			ActionServo* a = new ActionServo(gpioservo);
			a->setName(id);
			controller->registerSettings(a);
			controller->registerLoop(a);

		} else if (strcmp(art, "turnout") == 0) {
			ActionTurnOut* a = new ActionTurnOut(
					GPIO.string2gpio(value["dir1"].as<const char*>()),
					GPIO.string2gpio(value["dir2"].as<const char*>()),
					GPIO.string2gpio(value["enable"].as<const char*>()));
			a->setName(id);
			controller->registerSettings(a);
			controller->registerLoop(a);
		} else if (strcmp(art, "sendturnout") == 0) {
			int addr = value["addr"].as<int>();
			ActionSendTurnoutCommand* a = new ActionSendTurnoutCommand(controller, addr);
			a->setName(id);
			controller->registerSettings(a);
			controller->registerNotify(a);
		} else {
			Logger::getInstance()->addToLog(
					"Config: Unbekannter Eintrag " + String(art));
		}
		loop();

	}
}



void Config::parseCfg(Controller* controller, Webserver* web, JsonArray& r1) {
	for (auto value : r1) {
		if (ESP.getFreeHeap() < 1200) {
			lowmemory = true;
			break;
		}
		const char* art = (const char*) value["m"];
		if (art == NULL) {
			//Logger::getInstance()->addToLog("Null from json");
			continue;
		}
		if ((strcmp(art, "dcclogger") == 0) || (strcmp(art, "cmdlogger") == 0)) {
			controller->cmdlogger = new WebserviceCommandLogger();
			controller->registerNotify(controller->cmdlogger);
			web->registerWebServices(controller->cmdlogger);


		} else if (strcmp(art, "dccsniffer") == 0) {
			controller->dccSniffer = new WebserviceDCCSniffer();
			web->registerWebServices(controller->dccSniffer);


		} else if (strcmp(art, "dcc") == 0) {
			int gpio = GPIO.string2gpio(value["gpio"].as<const char*>());
			controller->registerCmdReceiver(new CmdReceiverDCC(controller, gpio, gpio));


		} else if (strcmp(art, "z21") == 0) {
			CmdReceiverZ21Wlan* rec = new CmdReceiverZ21Wlan(controller, value["ip"].as<const char*>());
			controller->registerCmdReceiver(rec);
			controller->registerCmdSender(rec);


		} else if (strcmp(art, "simulateZ21") == 0) {
			CmdZentraleZ21* rec = new CmdZentraleZ21(controller);
			controller->registerCmdReceiver(rec);


			//		} else if (strcmp(art, "espnow") == 0) {
			//			String rolle = String(value["rolle"].as<const char*>());
			//			Serial.println("Rolle: " + rolle);
			//			CmdReceiverESPNOW* rec = new CmdReceiverESPNOW(controller, rolle);
			//			controller->registerCmdReceiver(rec);


		} else if (strcmp(art, "webservicewifiscanner") == 0) {
			web->registerWebServices(new WebserviceWifiScanner());


		} else if (strcmp(art, "webservicelog") == 0) {
			web->registerWebServices(new WebserviceLog());


			//		} else if (strcmp(art, "mp3") == 0) {
			////			int  addr = value["addr"].as<int>();
			////			int tx = GPIO.string2gpio(value["tx"].as<const char*>());
			////			int rx = GPIO.string2gpio(value["rx"].as<const char*>());
			////			controller->registerNotify(new ActionDFPlayerMP3(addr, tx, rx));

		} else if (strcmp(art, "wlan") == 0) {
			WiFi.enableSTA(true);
			if (value["ip"] != NULL || value["netmask"] != NULL || value["gw"] != NULL) {
				if (value["ip"] != NULL && value["netmask"] != NULL && value["gw"] != NULL) {
					IPAddress ip;
					IPAddress nm;
					IPAddress gw;
					ip.fromString(value["ip"].as<const char*>());
					nm.fromString(value["netmask"].as<const char*>());
					gw.fromString(value["gw"].as<const char*>());
					WiFi.config(ip, gw, nm);
				} else {
					Logger::log("Netzwerkkonfiguration (ip, netmask, gw) unvollständig");
					continue;
				}
			} else {
				Logger::log("Netzwerkkonfiguration per DHCP");
			}
			int ch = 1;
			if (value["kanal"] != NULL) {
				ch = value["kanal"].as<int>();
			}
			WiFi.begin(value["ssid"].as<const char*>(), value["pwd"].as<const char*>(), ch);
		} else if (strcmp(art, "ap") == 0) {
			IPAddress Ip(192, 168, 0, 111);
			IPAddress NMask(255, 255, 255, 0);
			int ch = 1;
			if (value["kanal"] != NULL) {
				ch = value["kanal"].as<int>();
			}
			WiFi.softAPConfig(Ip, Ip, NMask);
			if (!WiFi.softAP(value["ssid"].as<const char*>(), value["pwd"].as<const char*>(), ch)) {
				Logger::log("softAP fehlgeschlagen!");
			}
			WiFi.enableAP(true);
			Serial.println(WiFi.softAPIP().toString());
			// TODO DNS -Server


		} else if (strcmp(art, "i2c") == 0) {
			int sda = GPIO.string2gpio(value["sda"].as<const char*>());
			int scl = GPIO.string2gpio(value["scl"].as<const char*>());
			Wire.begin(sda, scl);


		} else if (strcmp(art, "i2cslave") == 0) {
			const char* d = (const char*) value["d"];
			if (d != NULL && (strcmp(d, "MCP23017") == 0 || strcmp(d, "mcp23017") == 0)) {
				JsonArray& cfg = value["addr"];
				for (auto value : cfg) {
					int idx = value.as<int>();
					Wire.beginTransmission(idx + 0x20);
					int ret = Wire.endTransmission();
					String tret = "Failed (" + String(ret) + ")";
					if (ret == 0) {
						tret = "OK";
					}
					Logger::getInstance()->addToLog("Test MCP23017 auf I2c/" + String(idx + 0x20) + ": " + tret);
					if (ret == 0) {
						GPIO.addMCP23017(idx);
					}
				}
			} else {
				Logger::getInstance()->addToLog("Unbekanntes Gerät: " + String(d));
			}


		} else {
			Logger::getInstance()->addToLog(
					"Config: Unbekannter Eintrag " + String(art));
		}
		loop();
	}
}

void Config::parseConnector(Controller* controller, Webserver* web, JsonArray& r1) {
	int idx = 0;
	for (auto value : r1) {
		if (ESP.getFreeHeap() < 1200) {
			lowmemory = true;
			break;
		}
		idx++;
		const char* in = (const char*) value["in"];
		const char* out = (const char*) value["out"];
		String connectString = "conn" + String(idx) + "io";
		if (strcmp(in, "turnout") == 0 && strcmp(out, "led") == 0) {
			Connectors* cin;
			JsonArray& r1 = value["values"];
			for (auto value : r1) {
				String connectString = "conn" + String(idx) + "io"; idx++;
				if (ESP.getFreeHeap() < 1200) {
					lowmemory = true;
					break;
				}
				int addr = value[0].as<int>();
				const char* pin = value[1].as<const char*>();
				Logger::getInstance()->addToLog("Turnout/Led" + String(addr) + "/" + String(pin));


				Pin* ledgpio = new Pin(pin);
				ActionLed* l = new ActionLed(ledgpio);
				l->setName(connectString);
				controller->registerSettings(l);
				controller->registerLoop(l);


				ISettings* a = getSettingById(controller, connectString.c_str());
				cin = new ConnectorTurnout(a, addr);
				controller->registerNotify(cin);
			}


		} else if (strcmp(in, "gpio") == 0 && strcmp(out, "sendturnout") == 0) {
			Connectors* cin;
			JsonArray& r1 = value["values"];
			for (auto value : r1) {
				String connectString = "conn" + String(idx) + "io"; idx++;
				if (ESP.getFreeHeap() < 1200) {
					lowmemory = true;
					break;
				}
				const char* pin = value[0].as<const char*>();
				int addr = value[1].as<int>();
				Logger::getInstance()->addToLog("GPIO/Sendturnout " + String(pin) + "/" + String(addr));

				ActionSendTurnoutCommand* atc = new ActionSendTurnoutCommand(controller, addr);
				atc->setName(connectString);
				controller->registerSettings(atc);
				controller->registerNotify(atc);

				Pin* g = new Pin(pin);
				ISettings* a = getSettingById(controller, connectString.c_str());
				cin = new ConnectorGPIO(a, g);
				controller->registerNotify(cin);
			}

		} else {
			Logger::getInstance()->addToLog(
					"Config: Unbekannter Eintrag In: " + String(in) + " Out: " + String(out));
		}
	}
}


void Config::parseIn(Controller* controller, Webserver* web, JsonArray& r1) {
	for (JsonArray::iterator it = r1.begin(); it != r1.end(); ++it) {
		if (ESP.getFreeHeap() < 1200) {
			lowmemory = true;
			break;
		}
		JsonObject& value = *it;
		const char* art = (const char*) value["m"];
		if (art == NULL) {
			//Logger::getInstance()->addToLog("Null from json");
			continue;
		}
		Connectors* c;
		if (strcmp(art, "locospeed") == 0) {
			int l = value["addr"].as<int>();
			ISettings* a = getSettingById(controller, value["out"][0].as<const char*>());
			c = new ConnectorLocoSpeed(a, l);

		} else if (strcmp(art, "funconoff") == 0) {
			int l = value["addr"].as<int>();
			int f = value["func"].as<int>();
			ISettings* a = getSettingById(controller, value["out"][0].as<const char*>());
			c = new ConnectorONOFF(a, l, f);


		} else if (strcmp(art, "func2value") == 0) {
			int l = value["addr"].as<int>();
			JsonObject& fv = value["func2value"];
			int *array = new int[2 * fv.size()];
			int pos = 0;
			for (auto kv : fv) {
				array[pos++] = atoi(kv.key);
				array[pos++] = kv.value.as<int>();
			}
			ISettings* a = getSettingById(controller, value["out"][0].as<const char*>());
			c = new ConnectorFunc2Value(a, l, array, 2 * fv.size());


		} else if (strcmp(art, "turnout") == 0) {
			int addr = value["addr"].as<int>();
			ISettings* a = getSettingById(controller, value["out"][0].as<const char*>());
			c = new ConnectorTurnout(a, addr);


		} else if (strcmp(art, "lights") == 0) {
			int l = value["addr"].as<int>();
			int onoff = value["on"].as<int>();

			JsonArray& out = value["out"];
			if (out.size() != 2) {
				Logger::log("Keine zwei Ausgabe Led angegeben.");
				continue;
			}
			ISettings *ptr[4];
			for (int i = 0; i < 2; i++) {
				ptr[i] = getSettingById(controller, value["out"][i].as<const char*>());
			}
			Serial.println("Lights: " + String(onoff) + " " + " Addr: " + String(l));
			c = new ConnectorLights(ptr[0], ptr[1], l, onoff);
		} else if (strcmp(art, "gpio") == 0) {
			Pin* g = new Pin(value["gpio"].as<const char*>());
			ISettings* a = getSettingById(controller, value["out"][0].as<const char*>());
			c = new ConnectorGPIO(a, g);
		} else {
			Logger::getInstance()->addToLog(
					"Config: Unbekannter Eintrag " + String(art));
		}
		if (c != NULL) {
			controller->registerNotify(c);
		}
		loop();

	}
}

ISettings* Config::getSettingById(Controller* c, const char* id) {
	for (int idx = 0; idx < c->getSettings()->size(); idx++) {
		ISettings* s = c->getSettings()->get(idx);
		if (s->getName().equals(id)) {
			return s;
		}
	}
	Logger::getInstance()->addToLog("Config: Unbekannte ID " + String(id));
	return NULL;
}

int Config::jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}
boolean Config::lowmemory = false;
