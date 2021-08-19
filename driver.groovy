/**
 * Driver:     UPS Monitoring Service
 * Author:     Mirco Caramori
 * Repository: https://github.com/padus/vacation
 * Import URL: https://raw.githubusercontent.com/padus/vacation/main/driver.groovy
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 * for the specific language governing permissions and limitations under the License.
 *
 */

public static String version() { return "v1.0.9"; }

/**
 * Change Log:
 *
 * 2021.07.07 - Initial implementation
 * 2021.08.18 - Relocated repository: mircolino -> padus
 *
 */

// Metadata -------------------------------------------------------------------------------------------------------------------

metadata {
  definition(name: "UPS Monitoring Service", namespace: "mircolino", author: "Mirco Caramori", importUrl: "https://raw.githubusercontent.com/padus/vacation/main/driver.groovy") {
    capability "Sensor";
    capability "Presence Sensor";
    capability "Battery";

    // attribute "presence", "enum", ["present", "not present"];
    // attribute "battery", "number";
  }

  preferences {
    input(name: "macAddress", type: "string", title: "<font style='font-size:12px; color:#1a77c9'>MAC / IP Address</font>", description: "<font style='font-size:12px; font-style: italic'>Host where the UPS/battery is connected</font>", defaultValue: "", required: true);
    input(name: "logLevel", type: "enum", title: "<font style='font-size:12px; color:#1a77c9'>Log Verbosity</font>", description: "<font style='font-size:12px; font-style: italic'>Default: 'Debug' for 30 min and 'Info' thereafter</font>", options: [0:"Error", 1:"Warning", 2:"Info", 3:"Debug", 4:"Trace"], multiple: false, defaultValue: 3, required: true);
  }
}

// Preferences -----------------------------------------------------------------------------------------------------------------

private String macAddress() {
  //
  // Return the MAC or IP address as entered by the user, or an empty string if one hasn't been entered yet
  //
  if (settings.macAddress != null) return (settings.macAddress);
  return ("");
}

// -------------------------------------------------------------

private Integer logLevel() {
  //
  // Get the log level as an Integer:
  //
  //   0) log only Errors
  //   1) log Errors and Warnings
  //   2) log Errors, Warnings and Info
  //   3) log Errors, Warnings, Info and Debug
  //   4) log Errors, Warnings, Info, Debug and Trace (everything)
  //
  // If the level is not yet set in the driver preferences, return a default of 2 (Info)
  // Declared public because it's being used by the child-devices as well
  //
  if (settings.logLevel != null) return (settings.logLevel.toInteger());
  return (2);
}

// Logging ---------------------------------------------------------------------------------------------------------------------

private void logError(String str) { log.error(str); }
private void logWarning(String str) { if (logLevel() > 0) log.warn(str); }
private void logInfo(String str) { if (logLevel() > 1) log.info(str); }
private void logDebug(String str) { if (logLevel() > 2) log.debug(str); }
private void logTrace(String str) { if (logLevel() > 3) log.trace(str); }

// -------------------------------------------------------------

private void logResponse(String id, Object obj) {
  //
  // Log a generic groovy object
  // Used only for diagnostic/debug purposes
  //
  if (logLevel() > 3) {
    String text = id;
    obj.properties.each {
      text += "\n${it}";
    }
    logTrace(text);
  }
}

// -------------------------------------------------------------

private void logData(String id, Map data) {
  //
  // Log all data received from the device
  // Used only for diagnostic/debug purposes
  //
  if (logLevel() > 3) {
    String text = id;    
    data.each {
      text += "\n${it.key} = ${it.value}";
    }
    logTrace(text);    
  }
}

// -------------------------------------------------------------

void logDebugOff() {
  //
  // runIn() callback to disable "Debug" logging after 30 minutes
  // Cannot be private
  //
  if (logLevel() > 2) device.updateSetting("logLevel", [type: "enum", value: "2"]);
}

// DNI ------------------------------------------------------------------------------------------------------------------------

private Map dniIsValid(String str) {
  //
  // Return null if not valid
  // otherwise return both hex and canonical version
  //
  List<Integer> val = [];

  try {
    List<String> token = str.replaceAll(" ", "").tokenize(".:");
    if (token.size() == 4) {
      // Regular IPv4
      token.each {
        Integer num = Integer.parseInt(it, 10);
        if (num < 0 || num > 255) throw new Exception();
        val.add(num);
      }
    }
    else if (token.size() == 6) {
      // Regular MAC
      token.each {
        Integer num = Integer.parseInt(it, 16);
        if (num < 0 || num > 255) throw new Exception();
        val.add(num);
      }
    }
    else if (token.size() == 1) {
      // Hexadecimal IPv4 or MAC
      str = token[0];
      if ((str.length() != 8 && str.length() != 12) || str.replaceAll("[a-fA-F0-9]", "").length()) throw new Exception();
      for (Integer idx = 0; idx < str.length(); idx += 2) val.add(Integer.parseInt(str.substring(idx, idx + 2), 16));
    }
  }
  catch (Exception ignored) {
    val.clear();
  }

  Map dni = null;

  if (val.size() == 4) {
    dni = [:];
    dni.hex = sprintf("%02X%02X%02X%02X", val[0], val[1], val[2], val[3]);
    dni.canonical = sprintf("%d.%d.%d.%d", val[0], val[1], val[2], val[3]);
  }

  if (val.size() == 6) {
    dni = [:];
    dni.hex = sprintf("%02X%02X%02X%02X%02X%02X", val[0], val[1], val[2], val[3], val[4], val[5]);
    dni.canonical = sprintf("%02x:%02x:%02x:%02x:%02x:%02x", val[0], val[1], val[2], val[3], val[4], val[5]);
  }

  return (dni);
}

// Driver lifecycle ------------------------------------------------------------------------------------------------------------

void installed() {
  //
  // Called once when the driver is created
  //
  try {
    logDebug("installed()");
  }
  catch (Exception e) {
    logError("installed(): ${e.getMessage()}");
  }
}

// -------------------------------------------------------------

void updated() {
  //
  // Called everytime the user saves the driver preferences
  //
  try {
    logDebug("updated()");

    // Clear previous states
    state.clear();    

    // Unschedule possible previous runIn() calls
    unschedule();

    // Turn off debug log in 30 minutes
    if (logLevel() > 2) runIn(1800, logDebugOff);

    // Update DNI
    Map dni = dniIsValid(macAddress());
    if (!dni) throw new Exception("\"${macAddress()}\" is not a valid MAC or IP address");

    if (dni.hex != device.getDeviceNetworkId()) {
      device.setDeviceNetworkId(dni.hex);
    }
  }
  catch (Exception e) {
    logError("updated(): ${e.getMessage()}");
  }
}

// -------------------------------------------------------------

void uninstalled() {
  //
  // Called once when the driver is deleted
  //
  try {
    logDebug("uninstalled()");
  }
  catch (Exception e) {
    logError("uninstalled(): ${e.getMessage()}");
  }
}

// -------------------------------------------------------------

void parse(String msg) {
  //
  // Called everytime a GET/POST message is received from the WiFi Gateway
  //
  try {
    // Parse GET/POST message
    Map data = parseLanMessage(msg);

    // Log raw data received from the device if trace is enabled
    logData("parse()", data);

    if (data.containsKey("json")) {
      String val, desc;

      data["json"].each {
        switch (it.key) {
        case "mains":
          val = (it.value).toInteger()? "present": "not present";
          desc = "${device.getDisplayName()} mains is ${val}";

          sendEvent(name: "presence", value: val, linkText: device.getName(), descriptionText: desc);
          logInfo(desc);
          break;

        case "battery":
          val = it.value;
          desc = "${device.getDisplayName()} battery is ${val}%";

          sendEvent(name: "battery", value: val.toInteger(), unit: "%", linkText: device.getName(), descriptionText: desc);
          logInfo(desc);
          break;
        }
      }
    }
  }
  catch (Exception e) {
    logError("parse(): ${e.getMessage()}");
  }
}

// Recycle ---------------------------------------------------------------------------------------------------------------------

/*

*/

// EOF -------------------------------------------------------------------------------------------------------------------------
