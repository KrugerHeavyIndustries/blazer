This program provides command line access to [Backblaze](https://www.backblaze.com/b2/cloud-storage.html) 
B2 cloud storage service. 

It is currently an early work but is usable. 

Basic Usage

    blazer create_bucket
    blazer delete_bucket 
    blazer get_file_by_id <fileId> <localFileName>
    blazer get_file_by_name <bucketName> <fileName> <localFileName>
    blazer ls [-r]
    blazer upload_file
