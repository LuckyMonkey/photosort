# photosort
![screenshot](https://github.com/LuckyMonkey/photosort/blob/master/screenshot2.png)  
The idea is to quickly sort through a massive photo collection using vanilla php to filter metadata and use the effiency of typing to refine what cannot be filtered automatically.   
  
# BASICS
😲 PHP Syntax and EXIF functions are used to define statistics on each image.  
  
😲 Those Statistics can be used to help the user guide the images into a folder either automatically or by providing suggestions when unsure.  
  
😲 Each image in /images/ folder is to be sorted into one of the folders inside of /sorted/  
  
😲 Each folder in /sorted/ is listed on the page as a keyboard key to be pressed, When pressed the selected "highlighted" image is to moved into that folder, this also works as a button and be can clicked or touched.  

😲 The "z" key will be used as an "undo" function allowing the user to backup a step and correct mistakes  
  
😲 Images following the highlight image will be loaded as thumbnails and only so many will be displayed to the user in order to prevent the browser from crashing  

😲 EXIF Metadata about the image is displayed to the right of the highlighted image to tell the user different information such as date taken, location taken, camera used, if its a screenshot or not, dimentions, file size, file name, and color statistics  
  

# TODO (Future)
🛹 GPS EXIF will be resolved with an API or library to provide location names rather than GPS cordonates  
🛹 Allow many stages of "undo"  
🛹 Edit EXIF in form fields and have PHP save changed information  
🤕 Fix EXIF Metadata in broken images, handle errors  
🛹 Highlight and eliminate duplicates  
🤕 Allow multiable images to be selected at once for batch moving  
🛹 Allow sort folder sorting, display images as list or grid  
🤕 Support for video files will play in tiny player  
🛹 Gather statistics on metadata and destination folders and provide suggestions based on trends and similarities  
🛹 Button to create new folders to be sorted into  
🛹 Have each image output color statistics as a seed, and then compare to future images so if its a resized version of it PHP will be able to tell and say how many times its been encountered before.  
🤕 Link to cloud services (FB, Flikr, Instagram, Youtube, Google Drive, iCloud, OneDrive)  
🤕 Scrape cloud images and use color ID function on them to see if they're already hosted online or not  