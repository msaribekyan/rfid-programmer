use hex::decode;
use std::env::args;

enum Command {
    Detect,
    Read(u8, Vec<u8>),
    Write(u8, Vec<u8>, Vec<u8>),
}

enum ArgError {
    InvalidKey,
    InvalidData,
    InvalidBlock,
    InvalidCommand,
    NotEnoughArguments,
}

fn parse_hex(key_str: &str, exp_len: usize) -> Option<Vec<u8>> {
    match decode(key_str) {
        Ok(bytes) => {
            if bytes.len() == exp_len {
                Some(bytes)
            } else {
                None
            }
        }
        Err(_e) => None,
    }
}

fn parse_arg() -> Result<Command, ArgError> {
    let args: Vec<String> = args().collect();
    let arg_len = args.len();
    if arg_len <= 1 {
        return Err(ArgError::NotEnoughArguments);
    }
    match args[1].as_str() {
        "detect" => {
            if arg_len != 2 {
                return Err(ArgError::NotEnoughArguments);
            }
            Ok(Command::Detect)
        }
        "read" => {
            if arg_len != 4 {
                return Err(ArgError::NotEnoughArguments);
            }
            let blk: u8 = match args[2].parse() {
                Ok(blk) => {
                    if blk <= 15 {
                        blk
                    } else {
                        return Err(ArgError::InvalidBlock);
                    }
                }
                Err(_e) => return Err(ArgError::InvalidBlock),
            };
            let key = match parse_hex(&args[3], 6) {
                Some(bytes) => bytes,
                None => return Err(ArgError::InvalidKey),
            };
            Ok(Command::Read(blk, key))
        }
        "write" => {
            if arg_len != 5 {
                return Err(ArgError::NotEnoughArguments);
            }
            let blk: u8 = match args[2].parse() {
                Ok(blk) => {
                    if blk <= 15 {
                        blk
                    } else {
                        return Err(ArgError::InvalidBlock);
                    }
                }
                Err(_e) => return Err(ArgError::InvalidBlock),
            };
            let key = match parse_hex(&args[3], 6) {
                Some(bytes) => bytes,
                None => return Err(ArgError::InvalidKey),
            };
            let data = match parse_hex(&args[4], 16) {
                Some(bytes) => bytes,
                None => return Err(ArgError::InvalidData),
            };
            Ok(Command::Write(blk, key, data))
        }
        _ => Err(ArgError::InvalidCommand),
    }
}

fn main() {
    let arg = parse_arg();
    match arg {
        Ok(cmd) => {
            let ports = serialport::available_ports().expect("No ports found!");
            let mut port_name: Option<String> = None;
            for p in ports {
                match p.port_type {
                    serialport::SerialPortType::UsbPort(_) => {
                        port_name = Some(p.port_name);
                        break;
                    },
                    _ => {}
                };
            }
            let mut serial_port = match port_name {
                Some(p) => {
                        serialport::new(p, 115200).open().expect("Failed to open port")
                },
                None => return ,
            }; 
            match cmd {
                Command::Detect => {
                    let packet: Vec<u8> = vec![0x55, 0x01];
                    let _ = serial_port.write(&packet);
                    std::thread::sleep(std::time::Duration::from_millis(200));
                    let mut buffer = [0u8; 24];
                    let _ = serial_port.read(&mut buffer);
                    println!("Got {:02X?}", buffer);
                },
                Command::Read(blk, key) => {
                    let mut packet: Vec<u8> = vec![0x55, 0x02];
                    packet.push(blk);
                    packet.extend(key);
                    let _ = serial_port.write(&packet);
                    std::thread::sleep(std::time::Duration::from_millis(200));
                    let mut buffer = [0u8; 18];
                    let _ = serial_port.read(&mut buffer);
                    println!("Got {:02X?}", buffer);
                },
                Command::Write(blk, key, data) => { 
                    let mut packet: Vec<u8> = vec![0x55, 0x03];
                    packet.push(blk);
                    packet.extend(key);
                    packet.extend(data);
                    let _ = serial_port.write(&packet);
                    std::thread::sleep(std::time::Duration::from_millis(200));
                    let mut buffer = [0u8; 3];
                    let _ = serial_port.read(&mut buffer);
                    println!("Got {:02X?}", buffer);},
            }
        },
        Err(e) => match e {
            ArgError::InvalidKey => println!("Invalid key format"),
            ArgError::InvalidData => println!("Invalid data format"),
            ArgError::InvalidBlock => println!("Invalid block number"),
            ArgError::InvalidCommand => println!("Invalid command"),
            ArgError::NotEnoughArguments => println!("Not enough arguments"),
        },
    }
}
