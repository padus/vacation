# UPS Monitoring Service

Simple Windows application and Hubitat presence sensor driver to monitor AC presence and battery level of a UPS connected to a Windows system.

## Installation Instructions

### Hubitat Website:

1.  Add the UPS Monitoring Service [source code](https://raw.githubusercontent.com/mircolino/vacation/main/driver.groovy) to the Hubitat "Drivers Code" page.

2.  Create a new Virtual Device, select type: "UPS Monitoring Service" and press &lt;Save Device&gt;:

3.  Open the "UPS Monitoring Service" device page, enter the IP or MAC address of the Windows system where the UPS is connected and press &lt;Save Preferences&gt;:

<img src="https://github.com/mircolino/vacation/raw/main/images/device.png" width="50%" height="50%">

### Windows System:

Simply run the application with, as a parameter, the Hubitat IP address or hotname.

```text
  C:\> vacation.exe 192.168.10.19
```

The UPS Monitoring Service and the Hubitat Integration should now be fully operational.

<img src="https://github.com/mircolino/vacation/raw/main/images/events.png" width="50%" height="50%">

***

## Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
