# UPS Monitoring Service

Simple Windows application and Hubitat presence sensor driver to monitor AC presence and battery level of a UPS connected to a Windows system.

## Installation Instructions

### Hubitat Website

1. Add the UPS Monitoring Service [source code](https://raw.githubusercontent.com/mircolino/vacation/main/driver.groovy) to the Hubitat "Drivers Code" page.

2. Create a new Virtual Device, select type: "UPS Monitoring Service" and press &lt;Save Device&gt;:

3. Open the "UPS Monitoring Service" device page, enter the IP or MAC address of the Windows system where the UPS is connected and press &lt;Save Preferences&gt;:

   <img src="https://github.com/mircolino/vacation/raw/main/images/device.png" width=50% height=50%>

### Windows System

1. Create a new folder "C:\Program Files\UPS Monitoring Service" and copy the "vacation.exe", "syslog_register.reg" and "syslog_unregister.reg" files in it.\
   *NB: If change folder name you'll have to update the "syslog_register.reg" file and the commands below accordingly.*

2. Double-click the "syslog_register.reg" file to register the service log message provider.

3. Open a Powershell console as Administrator and execute the following commands, making sure to replace <hubitat_ip> with the Hubitat hub actual IP address or hostname:

   ```text
   PS C:\> New-Service -Name "vacation" -DisplayName "UPS Monitoring Service" -Description "Hubitat UPS AC power presence and battery percentage" -BinaryPathName "C:\Program Files\UPS Monitoring Service\vacation.exe <hubitat_ip>"
   PS C:\> Start-Service -Name "vacation"
   ```

4. The UPS Monitoring Service and the Hubitat Integration should now be fully operational.

   <img src="https://github.com/mircolino/vacation/raw/main/images/events.png" width="50%" height="50%">

5. To completely uninstall the service:

   - Open a Powershell console as Administrator and execute the following commands:

     ```text
     PS C:\> Stop-Service -Name "vacation"
     PS C:\> Remove-Service -Name "vacation"
     ```

   - Double-click the "syslog_unregister.reg" file to unregister the service log message provider.
   - Delete the folder "C:\Program Files\UPS Monitoring Service".

6. For troubleshooting, the service is logging errors and diagnostic to the System Event Viewer -> Windows Logs -> Application

***

## Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
