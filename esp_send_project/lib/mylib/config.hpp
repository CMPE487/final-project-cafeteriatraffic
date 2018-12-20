#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdint.h>

template<int type>
struct LoraMode{ };

// mode 1 (better reach, medium time on air)
template<>
struct LoraMode<1>{
  static const long bandwidth = 125E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 12;
};

// mode 2 (medium reach, less time on air)
template<>
struct LoraMode<2>{
  static const long bandwidth = 250E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 12;
};

// mode 3 (worst reach, less time on air)
template<>
struct LoraMode<3>{
  static const long bandwidth = 125E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 10;
};

// mode 4 (better reach, low time on air)
/* template<>
struct LoraMode<4>{
  static const long bandwidth = BW_500; // NOTE: not supported
  static const int codingrate = 5;
  static const int spreadingfactor = 12;
}; */

// mode 5 (better reach, medium time on air)
template<>
struct LoraMode<5>{
  static const long bandwidth = 250E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 10;
};

// mode 6 (better reach, worst time-on-air)
/* template<>
struct LoraMode<6>{
  static const long bandwidth = BW_500; // NOTE: not supported
  static const int codingrate = 5;
  static const int spreadingfactor = 11;
}; */

// mode 7 (medium-high reach, medium-low time-on-air)
template<>
struct LoraMode<7>{
  static const long bandwidth = 250E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 9;
};

// mode 8 (medium reach, medium time-on-air)
/* template<>
struct LoraMode<8>{
  static const long bandwidth = BW_500; // NOTE: not supported
  static const int codingrate = 5;
  static const int spreadingfactor = 9;
}; */

// mode 9 (medium-low reach, medium-high time-on-air)
/* template<>
struct LoraMode<9>{
  static const long bandwidth = BW_500; // NOTE: not supported
  static const int codingrate = 5;
  static const int spreadingfactor = 8;
}; */

// mode 10 (worst reach, less time_on_air)
/* template<>
struct LoraMode<10>{
  static const long bandwidth = BW_500; // NOTE: not supported
  static const int codingrate = 5;
  static const int spreadingfactor = 7;
}; */

// mode 11 (test / TOA: 1091ms, Link Budget: 160.3dB, Max Crystal Offset: 4.5ppm) // 9byte explicit header, CRC ON, optimizer OFF, 433MHz, 20dBm TX
template<>
struct LoraMode<11>{
  static const long bandwidth = 7.8E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 8;
};

// mode 12 (test / TOA: 3708ms, Link Budget: 165.6dB, Max Crystal Offset: 4.5ppm) // 9byte explicit header, CRC ON, optimizer OFF, preamble 6symbol, 433MHz, 20dBm TX
template<>
struct LoraMode<12>{
  static const long bandwidth = 7.8E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 10;
};

// mode 13 (test / TOA: 3702ms, Link Budget: 164.1dB, Max Crystal Offset: 18ppm) // 9byte explicit header, CRC ON, optimizer OFF, preamble 6symbol, 433MHz, 20dBm TX
template<> // NOTE: reached to my dorm room from Techneon
struct LoraMode<13>{
  static const long bandwidth = 31.25E3;
  static const int codingrate = 5;
  static const int spreadingfactor = 12;
};

// mode 14 (test / TOA: 4489ms, Link Budget: 164.1dB, Max Crystal Offset: 18ppm) // 9byte explicit header, CRC ON, optimizer OFF, preamble 6symbol, 433MHz, 20dBm TX
template<>
struct LoraMode<14>{
  static const long bandwidth = 31.25E3;
  static const int codingrate = 8;
  static const int spreadingfactor = 12;
};

// mode 15 (test / TOA: 561.15ms, Link Budget: 155.5dB, Max Crystal Offset: 72.2ppm) // 9byte explicit header, CRC ON, optimizer ON, preamble 6symbol, 433MHz, 20dBm TX
template<>
struct LoraMode<15>{
  static const long bandwidth = 125E3;
  static const int codingrate = 8;
  static const int spreadingfactor = 11;
};

// mode 16 (test / TOA: 12.86ms, Link Budget: 138dB, Max Crystal Offset: 288.7ppm) // 9byte explicit header, CRC ON, optimizer OFF, preamble 6symbol, 433MHz, 20dBm TX
/* template<>
struct LoraMode<16>{
  static const long bandwidth = BW_500; // NOTE: not supported
  static const int codingrate = 8;
  static const int spreadingfactor = 7;
}; */

// mode 17
template<>
struct LoraMode<17>{
  static const long bandwidth = 20.8E3;
  static const int codingrate = 8;
  static const int spreadingfactor = 12;
};

// mode 18
template<>
struct LoraMode<18>{
  static const long bandwidth = 31.25E3;
  static const int codingrate = 8;
  static const int spreadingfactor = 12;
};

#endif