# Blazer

![Maintained: yes](https://img.shields.io/badge/maintained-yes-brightgreen.svg)
[![Donate to this project using PayPal](https://img.shields.io/badge/paypal-donate-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=ULDAESU9WDWKG)
[![Donate to this project using Flattr](https://img.shields.io/badge/flattr-donate-green.svg)](https://flattr.com/profile/montdidier)

This program provides command line access to [Backblaze](https://www.backblaze.com/b2/cloud-storage.html) 
B2 cloud storage service.

It is currently an early work but is usable.

## Installation

### Ubuntu PPA

    sudo add-apt-repository ppa:chris.kruger+ubuntu/blazer
    sudo apt-get update

Then one can install the blazer application.

    sudo apt-get install blazer

### OSX

    brew tap KrugerHeavyIndustries/oss
    brew install blazer

## Setting Up

When you sign up with Backblaze you will be issued an account id and an
application key. You will need to inform blazer of those details in order to
use it. These details live in the blazer configuration file.

    mkdir ~/.blazer
    echo "accountId <YOUR_B2_ACCOUNT_ID>" > ~/.blazer/config
    echo "applicationKey <YOUR_B2_APP_KEY>" >> ~/blazer/config

## Basic Usage

    blazer (help)
    blazer create_bucket <bucketName>
    blazer delete_bucket <bucketName>
    blazer list_buckets
    blazer update_bucket <bucketName> [allPublic | allPrivate]
    blazer download_file_by_id <fileId> <localFileName>
    blazer download_file_by_name <bucketName> <remoteFileName> <localFileName>
    blazer delete_file_version <fileName> <fileId>
    blazer get_file_info <fileId>
    blazer hide_file <bucketName> <fileName>
    blazer ls <bucketName>
    blazer list_file_versions <bucketName> <fileName>
    blazer upload_file [-t <contentType>] <bucketName> <localFilePath> <remoteFilePath>
