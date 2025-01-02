// Copyright 2024, The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//! Rust structured logging API integration tests.
use structured_log::structured_log;
use structured_log::StructuredLogSection::SubsectionEnd;
use structured_log::StructuredLogSection::SubsectionStart;
use structured_log::LOG_ID_EVENTS;

const TAG_KEY_SAMPLE: u32 = 0x12345;

#[test]
fn log_i32() {
    let res = structured_log!(TAG_KEY_SAMPLE, 5i32);
    assert_eq!(res, Ok(()));
}

#[test]
fn log_i64() {
    let res = structured_log!(log_id: LOG_ID_EVENTS, TAG_KEY_SAMPLE, 5i64);
    assert_eq!(res, Ok(()));
}

#[test]
fn log_f32() {
    let res = structured_log!(TAG_KEY_SAMPLE, 5f32);
    assert_eq!(res, Ok(()));
}

#[test]
fn log_str() {
    let res = structured_log!(TAG_KEY_SAMPLE, "test string");
    assert_eq!(res, Ok(()));
}

#[test]
fn log_two_entries() {
    let res = structured_log!(TAG_KEY_SAMPLE, "test message", 10f32);
    assert_eq!(res, Ok(()));
}

#[test]
fn log_multiple_entries() {
    let res = structured_log!(TAG_KEY_SAMPLE, "test message", 10f32, 4i32, 4i32, 4i32, 4i32);
    assert_eq!(res, Ok(()));
}

#[test]
fn log_subsection() {
    let res = structured_log!(TAG_KEY_SAMPLE, SubsectionStart, "test message", SubsectionEnd);
    assert_eq!(res, Ok(()));
}

#[test]
fn log_subsection_start_end_swapped() {
    let res = structured_log!(TAG_KEY_SAMPLE, SubsectionEnd, "test message", SubsectionStart);
    assert_eq!(res, Err("unable to log value"));
}

#[test]
fn log_subsection_missing_start() {
    let res = structured_log!(TAG_KEY_SAMPLE, "test message", SubsectionEnd);
    assert_eq!(res, Err("unable to log value"));
}

#[test]
fn log_subsection_missing_end() {
    let res = structured_log!(TAG_KEY_SAMPLE, SubsectionStart, "test message");
    assert_eq!(res, Err("unable to write log message"));
}

#[test]
fn log_two_subsection() {
    let res = structured_log!(
        TAG_KEY_SAMPLE,
        SubsectionStart,
        5i32,
        SubsectionEnd,
        SubsectionStart,
        "test message",
        SubsectionEnd
    );
    assert_eq!(res, Ok(()));
}

#[test]
fn log_simple_entry_and_subsection() {
    let res = structured_log!(
        TAG_KEY_SAMPLE,
        "test message",
        17i64,
        SubsectionStart,
        10f32,
        SubsectionEnd
    );
    assert_eq!(res, Ok(()));
}

#[test]
fn mixed_subsection() {
    let res = structured_log!(
        TAG_KEY_SAMPLE,
        SubsectionStart,
        10f32,
        SubsectionStart,
        2i32,
        3i32,
        SubsectionEnd,
        SubsectionStart,
        20i32,
        30i32,
        SubsectionEnd,
        SubsectionEnd
    );
    assert_eq!(res, Ok(()));
}
