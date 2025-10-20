# SeisPCB v2025.10.20
A device used to detect seismic tremors and classify them by intensity on a custom intensity scale. Currently being built on a breadboard, I am planning to make it into one PCB.
The device utilizes a microprocessor (currently prototyping on an Arduino Nano), an ADXL345 module, and a A 0.96-inch TTL OLED module.
The scale goes as follows:

| Scale      | Range          | Abbreviated  |
| :--------- | :------------- | :----------: |
| Minimal    | x < 0.05 g     | MNML         |
| Weak       | 0.05 < x < 0.2 | WEAK         |
| Moderate   | 0.2 < x < 0.4  | MDRT         |
| Strong     | 0.4 < x < 0.7  | STRG         |
| Severe     | x > 0.7        | SEVR         |

