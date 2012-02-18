package com.androidmontreal.tododetector.service;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.channels.FileChannel;
import java.util.Date;

import com.androidmontreal.tododetector.db.ImageUploadHistoryDatabase.ImageUploadHistory;
import com.androidmontreal.tododetector.pref.PreferenceConstants;
import com.androidmontreal.tododetector.ui.MainPortal;

import com.androidmontreal.tododetector.R;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.database.Cursor;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.MediaStore.Images.Media;
import android.provider.Settings;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class TakePicture extends Activity {
	private static final int GPS_ENABLE = 2;
	private static final int TOOK_A_PICTURE = 0;
	Uri myPicture = null;
	Uri mImageDBUri = null;
	String mImageFilename = "";
	private LocationManager locationManager;
	private double longitude;
	private double latitude;



	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.take_picture);

		try {
			mImageFilename = getIntent().getExtras().getString(
					PreferenceConstants.EXTRA_IMAGEFILE_FULL_PATH);
		} catch (Exception e) {
			// TODO: handle exception

		}
		mImageDBUri = getIntent().getData();
		if (mImageFilename == null) {
			mImageFilename = "/sdcard/BacteriaCounting/watersamples/error.jpg";
		}
		if (mImageDBUri == null) {
			// This activity needs to be called with a URI of its
			// corresponding
			// row in the database.
			finish();
		}

		locationManager = (LocationManager) this
				.getSystemService(Context.LOCATION_SERVICE);
		if (!locationManager
		.isProviderEnabled(android.location.LocationManager.GPS_PROVIDER)) {
			Intent myIntent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
			startActivityForResult(myIntent, GPS_ENABLE);
		}else{
			getGeoTagAndPicture();
		}


		
		
	}

	

	public void onCaptureImageClick(View view) {
		getGeoTagAndPicture();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		switch (requestCode) {
		case GPS_ENABLE:
			if (!locationManager
					.isProviderEnabled(android.location.LocationManager.GPS_PROVIDER)) {
				Toast.makeText(getApplicationContext(),
						"Need the GPS in order to GeoTag picture", Toast.LENGTH_LONG).show();
				finish();
			}

			getGeoTagAndPicture();

			break;
		case TOOK_A_PICTURE:

			if(resultCode == Activity.RESULT_OK){
				// Now we know that our myPicture URI refers to the image just
				// taken
				/*
				 * copy image to results folder
				 */
				try {
					File sd = Environment.getExternalStorageDirectory();
					if (sd.canWrite()) {
						String sourceImagePath = getPath(myPicture);
						String destinationImagePath = mImageFilename;
						File source = new File(sourceImagePath);
						File destination = new File(destinationImagePath);
						if (source.exists()) {
							FileChannel src = new FileInputStream(source)
							.getChannel();
							FileChannel dst = new FileOutputStream(destination)
							.getChannel();
							dst.transferFrom(src, 0, src.size());
							src.close();
							dst.close();
						}
					}
					int affectedEntriesCount = updateImageMetadata(mImageDBUri);
					Toast.makeText(
							getApplicationContext(),
							"Saving as " + mImageFilename + "\nUpdated "
									+ affectedEntriesCount + " water sample.",
									Toast.LENGTH_LONG).show();
					finish();
				} catch (Exception e) {
					Toast.makeText(
							getApplicationContext(),
							"Result picture wasn't copied, but it's in the Camera folder: "
									+ getPath(myPicture), Toast.LENGTH_LONG)
									.show();
				}

			}
			finish();
			break;

		default:
			//finish();
			break;

		}

	}
	private void getGeoTagAndPicture(){
		// Define a listener that responds to location updates
		LocationListener locationListener = new LocationListener() {
			public void onLocationChanged(Location location) {
				// Called when a new location is found by the network location
				// provider.
				makeUseOfNewLocation(location);
			}

			public void onStatusChanged(String provider, int status,
					Bundle extras) {
			}

			public void onProviderEnabled(String provider) {
			}

			public void onProviderDisabled(String provider) {
			}
		};

		// Register the listener with the Location Manager to receive location
		// updates
		locationManager.requestLocationUpdates(
				LocationManager.NETWORK_PROVIDER, 0, 0, locationListener);

		ContentValues values = new ContentValues();
		values.put(Media.TITLE, mImageFilename);
		values.put(Media.DESCRIPTION,
				"Image Captured as part of Bacteria Counting Water Sample");

		myPicture = getContentResolver().insert(Media.EXTERNAL_CONTENT_URI,
				values);
		Intent i = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
		i.putExtra(MediaStore.EXTRA_OUTPUT, myPicture);

		startActivityForResult(i, TOOK_A_PICTURE);
	}
	
	/**
	 * TODO detect GPS on device, turn it on and get the Latitude and Longitude
	 * when this image is shot.
	 * 
	 * @param uri
	 *            which matches the row in the database for this image
	 * @return
	 */
	private int updateImageMetadata(Uri uri) {
		Date now = new Date();
		String metadataInJSON = String.format("{lat: %f, long: %f, timestamp:%d, user: %d}", latitude, longitude, now.getTime(), 12345);
		ContentValues values = new ContentValues();
		values.put(ImageUploadHistory.FILEPATH, mImageFilename);
		values.put(ImageUploadHistory.UPLOADED, "0");// sets deleted flag to
		// true
		values.put(ImageUploadHistory.METADATA, metadataInJSON);
		return getContentResolver().update(uri, values, null, null);
	}

	public String getPath(Uri uri) {
		String[] projection = { MediaStore.Images.Media.DATA };
		Cursor cursor = managedQuery(uri, projection, null, null, null);
		startManagingCursor(cursor);
		int column_index = cursor
				.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
		cursor.moveToFirst();
		return cursor.getString(column_index);
	}

	

	protected void makeUseOfNewLocation(Location location) {
		longitude = location.getLongitude();
		latitude = location.getLatitude();
	}


}