#!/bin/sh

A=${1#$2}
echo ${A#/}
