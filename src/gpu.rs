use napi::bindgen_prelude::Null;
use std::fmt::{self, Display};
use std::ffi::CStr;

#[allow(non_camel_case_types)]
pub mod ffi {
  use std::ffi::c_void;
  use std::os::raw::c_char;
  
  #[repr(C)]
  #[derive(Copy, Clone, Debug)]
  pub struct gpu_vk_device {
    pub id: u32,
    pub vktype: i32,
    pub name: [c_char; 256],
    pub under_ver_1_1: bool, // internal
  }

  extern "C" {

    pub fn gpu_init_volk();
    pub fn gpu_get_supported_devices(count: *mut i32, devices: *mut *mut gpu_vk_device);
  }

}

#[napi(js_name = "getGPUs")]
pub fn gpu_get_supported_devices() -> Vec<GPU> {

  let mut gpus: Vec<GPU> = Vec::new();

  unsafe {

    let mut count: i32 = 0;
    let mut devicesptr: *mut ffi::gpu_vk_device = std::ptr::null_mut();

    ffi::gpu_get_supported_devices(&mut count, &mut devicesptr);

    let devices = std::slice::from_raw_parts(devicesptr, count as usize);

    for i in 0..count as usize {
      gpus.push(GPU{
        id: devices[i].id,
        kind: String::new(), // TODO
        name: CStr::from_ptr(devices[i].name.as_ptr()).to_str().unwrap().to_owned(),
        under_ver_1_1: devices[i].under_ver_1_1
      });
    }
  };

  gpus
}

#[napi]
pub struct GPU {
  pub id: u32,
  pub kind: String,
  pub name: String,
  pub under_ver_1_1: bool
}

#[napi]
impl GPU {
  #[napi(getter)]
  pub fn get_name(&self) -> String {
    self.name.clone()
  }

  #[napi(getter)]
  pub fn get_kind(&self) -> String {
    self.kind.clone()
  }
}