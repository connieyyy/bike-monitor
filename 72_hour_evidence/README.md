# BikeBeacon T04 72-hour run

The run window started **Friday, Nov 20, 2026 at 7:20:39.456 p.m. Pacific** and ended **Monday, Nov 23 at 7:20:39.456 p.m. Pacific**. BikeBeacon sent telemetry from the ESP32 over Wi-Fi to EMQX, and Node-RED recorded it on Charlie's Mac. The log covers the full test window and includes interruptions and recoveries.

## Results

| Measure | Result |
| --- | ---: |
| Elapsed test window | 72 hours |
| Nominal heartbeat count at one message every two seconds | 129,600 |
| Telemetry messages Node-RED recorded during the window | 126,790 |
| Estimated ESP32 uptime from counters across five boots | 70h 56m 7s |
| Time between samples across the five receive gaps of 10 seconds or more | 1h 6m 12.083s |
| Estimated telemetry coverage using that 10-second gap threshold | 70h 53m 47.917s (98.47%) |

The heartbeat count assumes a steady two-second cadence. The firmware also sends messages on state changes and disarm confirmations, which can shift the heartbeat schedule. The report can compare 126,790 recorded messages with the nominal baseline. An exact packet-loss count would require evidence of what the board attempted to send. [Firmware publish logic](../src/bike_cloud.cpp)

The uptime estimate uses the change in `uptime_s` within each boot. It subtracts the first sample's uptime from the first boot and uses the last observed uptime for later boots. The resulting sum covers time evidenced by the counters. Telemetry coverage uses a separate measure. It subtracts the intervals of 10 seconds or more between consecutive receive timestamps from the 72-hour window. Delayed samples can arrive after an interruption, as they did after the second laptop power loss.

## Interruptions and delayed samples

The table lists adjacent receive intervals of at least six seconds. Times are Pacific, and each interval goes from the last received sample to the first sample after it. The five intervals of 10 seconds or more are included in the coverage estimate above. [Raw timestamps and sequence numbers](telemetry.jsonl)

| Date | Last sample → next sample | Interval | What we know |
| --- | --- | ---: | --- |
| Fri, Nov 20 | 8:06:43.669 p.m. → 8:06:50.476 p.m. | 6.807s | Cause unverified. Sequence continued. |
| Sun, Nov 22 | 2:07:00.500 a.m. → 2:07:07.501 a.m. | 7.001s | Cause unverified. Sequence continued. |
| Sun, Nov 22 | 5:09:06.708 p.m. → 5:17:37.524 p.m. | 8m 30.816s | Charlie confirmed the Mac lost power. The ESP32 uptime counter also reset. |
| Sun, Nov 22 | 10:29:15.232 p.m. → 11:17:07.847 p.m. | 47m 52.615s | The Mac and ESP32 moved to a different Wi-Fi network. Telemetry resumed after the board rejoined Wi-Fi following provisioning. |
| Mon, Nov 23 | 10:44:59.402 a.m. → 10:53:21.110 a.m. | 8m 21.708s | The setup returned to the original home Wi-Fi. The ESP32 uptime counter reset. The exact reset action is unverified. |
| Mon, Nov 23 | 3:33:51.027 p.m. → 3:35:06.266 p.m. | 1m 15.239s | Charlie confirmed the Mac lost power. Delayed samples arrived in a burst on recovery. |
| Mon, Nov 23 | 4:52:52.185 p.m. → 4:53:03.890 p.m. | 11.705s | Cause unverified. Sequence continued. |
| Mon, Nov 23 | 4:53:04.265 p.m. → 4:53:11.953 p.m. | 7.688s | Cause unverified. Sequence continued. |

## Evidence files

Use [the official telemetry export](telemetry.jsonl) for run calculations. It has **126,790 rows** and covers the confirmed window, with the start timestamp included and the end timestamp excluded. The last sample arrived at **7:20:39.101 p.m. Pacific** on Monday, 0.355 seconds before the end.

The [manual dashboard download](telemetry_with_padding.jsonl) has **129,260 rows**. All 126,790 official rows match byte for byte and in order. The download also includes **1,748 preflight rows** and **722 post-run rows**. These 2,470 extra rows account for all **474,586 extra bytes**. The dashboard button exports the recorder's full history through the download time.

The event files contain the [connection log](connection.jsonl), [motion log](motion.jsonl), [armed-state log](system.jsonl), [alarm log](alarm.jsonl), and [disarm attempts](attempts.jsonl). Use the event logs alongside the telemetry timestamps. The recorder can observe a connection change later than the start of a gap in telemetry.

## Current draw for the final report

The Progress Report predicted **7.9 mA** average draw and about **72 hours on 569 mAh**, assuming a 10-second heartbeat and 95% deep sleep. The tested firmware ran continuously with a two-second heartbeat. The evidence contains no current measurement. For the final report, record USB input voltage, average current, and the device state during measurement. Label a later reading as a post-run measurement.
